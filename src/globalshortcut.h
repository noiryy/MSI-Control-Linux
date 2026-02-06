#pragma once

#include <QObject>
#include <QKeySequence>

class GlobalShortcut : public QObject {
    Q_OBJECT

public:
    explicit GlobalShortcut(QObject *parent = nullptr);
    ~GlobalShortcut() override;

    bool isSupported() const;
    void setShortcut(const QKeySequence &sequence);

signals:
    void activated();

private:
    void unregisterShortcut();
    bool registerShortcut(const QKeySequence &sequence);

    QKeySequence sequence_;
    bool supported_ = false;
};
