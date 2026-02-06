#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QProcess>
#include <QHash>

class QLabel;
class QPushButton;
class QKeySequenceEdit;
class QShortcut;
class QSystemTrayIcon;
class QCheckBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void setScenario(const QString &mode);
    void toggleCoolerBoost();
    void applyTheme(bool darkMode);
    void updateShortcut(const QString &mode, const QKeySequence &sequence);
    void handleProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void buildUi();
    void refreshState();
    QString readSysfsValue(const QString &path) const;
    void notify(const QString &title, const QString &message);
    void runCommand(const QString &command, const QString &successMessage);

    QWidget *centralWidget_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QPushButton *coolerBoostButton_ = nullptr;
    QCheckBox *themeToggle_ = nullptr;
    QSystemTrayIcon *trayIcon_ = nullptr;

    QMap<QString, QPointer<QShortcut>> shortcuts_;
    QHash<QProcess *, QString> pendingMessages_;
};
