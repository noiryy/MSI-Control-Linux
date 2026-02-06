#include "globalshortcut.h"

#include <QGuiApplication>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <algorithm>

#ifdef MSI_CTL_HAS_X11
#include <QAbstractNativeEventFilter>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#endif

namespace {
#ifdef MSI_CTL_HAS_X11
class X11ShortcutFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        if (eventType != "xcb_generic_event_t") {
            return false;
        }

        auto *event = static_cast<xcb_generic_event_t *>(message);
        const uint8_t responseType = event->response_type & 0x7f;
        if (responseType != XCB_KEY_PRESS) {
            return false;
        }

        auto *keyEvent = reinterpret_cast<xcb_key_press_event_t *>(event);
        const quint32 keycode = keyEvent->detail;
        const quint32 modifiers = keyEvent->state;

        for (auto &entry : shortcuts_) {
            if (entry.keycode == keycode && (modifiers & entry.modifiers) == entry.modifiers) {
                emit entry.target->activated();
                if (result) {
                    *result = 1;
                }
                return true;
            }
        }

        return false;
    }

    struct ShortcutEntry {
        quint32 keycode;
        quint32 modifiers;
        GlobalShortcut *target;
    };

    QVector<ShortcutEntry> shortcuts_;
};

X11ShortcutFilter *filterInstance() {
    static X11ShortcutFilter *filter = nullptr;
    if (!filter) {
        filter = new X11ShortcutFilter();
    }
    return filter;
}

quint32 x11Modifiers(const Qt::KeyboardModifiers modifiers) {
    quint32 x11Mods = 0;
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        x11Mods |= ShiftMask;
    }
    if (modifiers.testFlag(Qt::ControlModifier)) {
        x11Mods |= ControlMask;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        x11Mods |= Mod1Mask;
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        x11Mods |= Mod4Mask;
    }
    return x11Mods;
}
#endif
} // namespace

GlobalShortcut::GlobalShortcut(QObject *parent)
    : QObject(parent) {
#ifdef MSI_CTL_HAS_X11
    if (QGuiApplication::platformName().contains("xcb")) {
        supported_ = true;
        QGuiApplication::instance()->installNativeEventFilter(filterInstance());
    }
#endif
}

GlobalShortcut::~GlobalShortcut() {
    unregisterShortcut();
}

bool GlobalShortcut::isSupported() const {
    return supported_;
}

void GlobalShortcut::setShortcut(const QKeySequence &sequence) {
    unregisterShortcut();
    sequence_ = sequence;
    if (!sequence_.isEmpty()) {
        registerShortcut(sequence_);
    }
}

void GlobalShortcut::unregisterShortcut() {
#ifdef MSI_CTL_HAS_X11
    if (!supported_ || sequence_.isEmpty()) {
        return;
    }

    Display *display = QX11Info::display();
    if (!display) {
        return;
    }

    auto *filter = filterInstance();
    filter->shortcuts_.erase(std::remove_if(filter->shortcuts_.begin(), filter->shortcuts_.end(),
                                           [this](const X11ShortcutFilter::ShortcutEntry &entry) {
                                               return entry.target == this;
                                           }),
                            filter->shortcuts_.end());

    const int keycode = XKeysymToKeycode(display, static_cast<KeySym>(sequence_[0].key()));
    if (keycode != 0) {
        XUngrabKey(display, keycode, AnyModifier, DefaultRootWindow(display));
        XSync(display, False);
    }
#endif
}

bool GlobalShortcut::registerShortcut(const QKeySequence &sequence) {
#ifdef MSI_CTL_HAS_X11
    if (!supported_) {
        return false;
    }

    Display *display = QX11Info::display();
    if (!display) {
        supported_ = false;
        return false;
    }

    const Qt::Key key = sequence[0].key();
    const Qt::KeyboardModifiers modifiers = sequence[0].keyboardModifiers();
    const quint32 x11Mods = x11Modifiers(modifiers);
    const KeySym keySym = static_cast<KeySym>(key);
    const int keycode = XKeysymToKeycode(display, keySym);
    if (keycode == 0) {
        return false;
    }

    const Window root = DefaultRootWindow(display);
    XGrabKey(display, keycode, x11Mods, root, True, GrabModeAsync, GrabModeAsync);
    XSync(display, False);

    auto *filter = filterInstance();
    filter->shortcuts_.push_back({static_cast<quint32>(keycode), x11Mods, this});
    return true;
#else
    Q_UNUSED(sequence);
    return false;
#endif
}
