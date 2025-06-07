#include "globalhotkeymanager.h"
#include <QApplication>

#ifdef Q_OS_WIN

static UINT qtKeyToVK(Qt::Key key)
{
    if (key >= Qt::Key_A && key <= Qt::Key_Z)
        return 'A' + (key - Qt::Key_A);
    if (key >= Qt::Key_0 && key <= Qt::Key_9)
        return '0' + (key - Qt::Key_0);
    return 0;
}

GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent)
    : QObject(parent)
{
    qApp->installNativeEventFilter(this);
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    for (auto id : registrations.keys())
        UnregisterHotKey(nullptr, id);
    qApp->removeNativeEventFilter(this);
}

void GlobalHotkeyManager::registerHotkey(int id, const QKeySequence &sequence)
{
    if (sequence.count() == 0)
        return;
    int keyInt = sequence[0];
    Qt::KeyboardModifiers mods = Qt::KeyboardModifiers(keyInt & Qt::KeyboardModifierMask);
    Qt::Key key = Qt::Key(keyInt & ~Qt::KeyboardModifierMask);

    UINT nativeMods = 0;
    if (mods & Qt::ShiftModifier)
        nativeMods |= MOD_SHIFT;
    if (mods & Qt::ControlModifier)
        nativeMods |= MOD_CONTROL;
    if (mods & Qt::AltModifier)
        nativeMods |= MOD_ALT;
    if (mods & Qt::MetaModifier)
        nativeMods |= MOD_WIN;

    UINT vk = qtKeyToVK(key);
    if (vk == 0)
        return;

    RegisterHotKey(nullptr, id, nativeMods, vk);
    registrations[id] = qMakePair(nativeMods, vk);
}

bool GlobalHotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY) {
        emit activated(static_cast<int>(msg->wParam));
        if (result)
            *result = 0;
        return true;
    }
    return false;
}

#else // Non-Windows

GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent)
    : QObject(parent)
{
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
}

void GlobalHotkeyManager::registerHotkey(int, const QKeySequence &)
{
}

#endif

