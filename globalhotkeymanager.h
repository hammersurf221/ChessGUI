#ifndef GLOBALHOTKEYMANAGER_H
#define GLOBALHOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>
#ifdef Q_OS_WIN
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <windows.h>
#endif

class GlobalHotkeyManager : public QObject
#ifdef Q_OS_WIN
    , public QAbstractNativeEventFilter
#endif
{
    Q_OBJECT
public:
    explicit GlobalHotkeyManager(QObject *parent = nullptr);
    ~GlobalHotkeyManager();

    void registerHotkey(int id, const QKeySequence &sequence);

signals:
    void activated(int id);

#ifdef Q_OS_WIN
protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    QMap<int, QPair<UINT, UINT>> registrations;
#endif
};

#endif // GLOBALHOTKEYMANAGER_H
