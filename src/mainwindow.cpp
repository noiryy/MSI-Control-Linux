#include "mainwindow.h"

#include <QCheckBox>
#include <QFont>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    buildUi();
    applyTheme(true);
}

void MainWindow::buildUi() {
    centralWidget_ = new QWidget(this);
    centralWidget_->setObjectName("central");
    setCentralWidget(centralWidget_);

    auto *mainLayout = new QVBoxLayout(centralWidget_);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto *title = new QLabel("MSI Control Center", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *subtitle = new QLabel("Quick scenarios, keybinds, and cooler boost.", this);
    subtitle->setWordWrap(true);

    themeToggle_ = new QCheckBox("Dark mode", this);
    themeToggle_->setChecked(true);
    connect(themeToggle_, &QCheckBox::toggled, this, &MainWindow::applyTheme);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(themeToggle_);

    auto *scenarioGroup = new QGroupBox("User scenarios", this);
    auto *scenarioLayout = new QVBoxLayout(scenarioGroup);
    scenarioLayout->setSpacing(12);

    struct ScenarioConfig {
        QString mode;
        QString label;
        QKeySequence defaultShortcut;
    };

    const QVector<ScenarioConfig> scenarios{
        {"eco", "Eco", QKeySequence("Ctrl+1")},
        {"comfort", "Comfort", QKeySequence("Ctrl+2")},
        {"turbo", "Turbo", QKeySequence("Ctrl+3")},
    };

    for (const auto &scenario : scenarios) {
        auto *row = new QHBoxLayout();
        auto *button = new QPushButton(scenario.label, this);
        button->setProperty("scenario", scenario.mode);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, [this, scenario]() { setScenario(scenario.mode); });

        auto *sequenceEdit = new QKeySequenceEdit(this);
        sequenceEdit->setKeySequence(scenario.defaultShortcut);
        connect(sequenceEdit, &QKeySequenceEdit::keySequenceChanged, this,
                [this, scenario](const QKeySequence &sequence) { updateShortcut(scenario.mode, sequence); });

        row->addWidget(button);
        row->addStretch();
        row->addWidget(new QLabel("Shortcut:", this));
        row->addWidget(sequenceEdit);
        scenarioLayout->addLayout(row);

        auto *shortcut = new QShortcut(scenario.defaultShortcut, this);
        connect(shortcut, &QShortcut::activated, this, [this, scenario]() { setScenario(scenario.mode); });
        shortcuts_.insert(scenario.mode, shortcut);
    }

    auto *coolerGroup = new QGroupBox("Cooler boost", this);
    auto *coolerLayout = new QVBoxLayout(coolerGroup);
    coolerBoostButton_ = new QPushButton("Enable cooler boost", this);
    coolerBoostButton_->setCheckable(true);
    coolerBoostButton_->setCursor(Qt::PointingHandCursor);
    connect(coolerBoostButton_, &QPushButton::clicked, this, &MainWindow::toggleCoolerBoost);
    coolerLayout->addWidget(coolerBoostButton_);

    statusLabel_ = new QLabel("Ready to switch scenarios.", this);
    statusLabel_->setWordWrap(true);

    auto *infoLabel = new QLabel(
        "Root permission is required for every action. You may be prompted by your system's "
        "authentication dialog.",
        this);
    infoLabel->setWordWrap(true);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(subtitle);
    mainLayout->addWidget(scenarioGroup);
    mainLayout->addWidget(coolerGroup);
    mainLayout->addWidget(statusLabel_);
    mainLayout->addWidget(infoLabel);

    trayIcon_ = new QSystemTrayIcon(QIcon(":/assets/msi-ctl.svg"), this);
    trayIcon_->setToolTip("MSI Control");
    trayIcon_->show();

    setMinimumSize(520, 420);

    refreshState();
}

void MainWindow::applyTheme(bool darkMode) {
    const QString gradient = darkMode
        ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1e1e1e, stop:1 #3b3b3b)"
        : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #e4e4e4)";

    const QString textColor = darkMode ? "#f5f5f5" : "#1c1c1c";
    const QString cardColor = darkMode ? "rgba(255, 255, 255, 0.08)" : "rgba(255, 255, 255, 0.65)";
    const QString buttonColor = darkMode ? "#2f2f2f" : "#f2f2f2";
    const QString buttonHover = darkMode ? "#3a3a3a" : "#e9e9e9";
    const QString accent = darkMode ? "#8dc6ff" : "#2b6cb0";

    const QString style = QString(
        "QWidget#central { background: %1; color: %2; }"
        "QGroupBox { border: 1px solid rgba(255, 255, 255, 0.15); border-radius: 18px; margin-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; }"
        "QLabel { color: %2; }"
        "QPushButton { background: %3; border-radius: 18px; padding: 8px 18px; border: 1px solid rgba(0,0,0,0.15); }"
        "QPushButton:hover { background: %4; }"
        "QPushButton:checked { background: %5; color: %2; border: 1px solid %5; }"
        "QCheckBox { spacing: 8px; }"
        "QLineEdit, QKeySequenceEdit { background: %3; border-radius: 12px; padding: 4px 8px; }")
                            .arg(gradient, textColor, buttonColor, buttonHover, accent, cardColor);

    centralWidget_->setStyleSheet(style);

    const QString groupStyle = QString("QGroupBox { background: %1; }").arg(cardColor);
    for (auto *group : findChildren<QGroupBox *>()) {
        group->setStyleSheet(groupStyle);
    }

    if (coolerBoostButton_) {
        coolerBoostButton_->setText(coolerBoostButton_->isChecked() ? "Disable cooler boost"
                                                                    : "Enable cooler boost");
    }
}

void MainWindow::setScenario(const QString &mode) {
    const QString command = QString("echo %1 | tee /sys/devices/platform/msi-ec/shift_mode").arg(mode);
    runCommand(command, QString("Scenario switched to %1.").arg(mode));
}

void MainWindow::toggleCoolerBoost() {
    const bool enabled = coolerBoostButton_->isChecked();
    const QString command =
        QString("echo %1 | tee /sys/devices/platform/msi-ec/cooler_boost").arg(enabled ? "on" : "off");
    runCommand(command, enabled ? "Cooler boost enabled." : "Cooler boost disabled.");
    coolerBoostButton_->setText(enabled ? "Disable cooler boost" : "Enable cooler boost");
}

void MainWindow::updateShortcut(const QString &mode, const QKeySequence &sequence) {
    if (shortcuts_.contains(mode) && shortcuts_[mode]) {
        shortcuts_[mode]->setKey(sequence);
    } else {
        auto *shortcut = new QShortcut(sequence, this);
        connect(shortcut, &QShortcut::activated, this, [this, mode]() { setScenario(mode); });
        shortcuts_.insert(mode, shortcut);
    }

    statusLabel_->setText(QString("Shortcut updated for %1.").arg(mode));
}

void MainWindow::refreshState() {
    const QString shiftMode = readSysfsValue("/sys/devices/platform/msi-ec/shift_mode");
    if (!shiftMode.isEmpty()) {
        statusLabel_->setText(QString("Current scenario: %1").arg(shiftMode));
    }

    const QString coolerBoost = readSysfsValue("/sys/devices/platform/msi-ec/cooler_boost");
    if (!coolerBoost.isEmpty()) {
        const bool enabled = coolerBoost.trimmed() == "on";
        coolerBoostButton_->setChecked(enabled);
        coolerBoostButton_->setText(enabled ? "Disable cooler boost" : "Enable cooler boost");
    }
}

QString MainWindow::readSysfsValue(const QString &path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    return stream.readAll().trimmed();
}

void MainWindow::runCommand(const QString &command, const QString &successMessage) {
    auto *process = new QProcess(this);
    pendingMessages_.insert(process, successMessage);

    connect(process, &QProcess::finished, this, &MainWindow::handleProcessFinished);

    process->start("pkexec", {"sh", "-c", command});
    statusLabel_->setText("Waiting for authentication...");
}

void MainWindow::handleProcessFinished(int exitCode, QProcess::ExitStatus status) {
    auto *process = qobject_cast<QProcess *>(sender());
    QString message = "Action failed.";

    if (process) {
        if (status == QProcess::NormalExit && exitCode == 0) {
            message = "Action completed successfully.";
        }

        if (pendingMessages_.contains(process) && status == QProcess::NormalExit && exitCode == 0) {
            message = pendingMessages_.value(process);
        }
    }

    statusLabel_->setText(message);
    notify("MSI Control", message);

    if (process) {
        pendingMessages_.remove(process);
        process->deleteLater();
    }

    refreshState();
}

void MainWindow::notify(const QString &title, const QString &message) {
    if (trayIcon_) {
        trayIcon_->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}
