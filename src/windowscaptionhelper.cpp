#include "windowscaptionhelper.h"
#include <QGuiApplication>
#include <QTimer>

#ifdef Q_OS_WINDOWS
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

WindowsCaptionHelper::WindowsCaptionHelper(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WINDOWS
    qApp->installNativeEventFilter(this);
#endif
}

WindowsCaptionHelper::~WindowsCaptionHelper()
{
#ifdef Q_OS_WINDOWS
    qApp->removeNativeEventFilter(this);
#endif
}

int WindowsCaptionHelper::captionHeight() const
{
    return mCaptionHeight;
}

void WindowsCaptionHelper::setCaptionHeight(int height)
{
    if (mCaptionHeight == height)
        return;

    mCaptionHeight = height;
    emit captionHeightChanged();
}

#ifdef Q_OS_WINDOWS

bool WindowsCaptionHelper::isFixedSize(QWindow *window)
{
    if (!window)
        return false;

    return window->minimumSize().isValid()
           && window->maximumSize().isValid()
           && window->minimumSize() == window->maximumSize();
}

void WindowsCaptionHelper::attachTo(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd || mWindows.contains(hwnd))
        return;

    WindowData data;
    data.window = window;
    mWindows.insert(hwnd, data);

    applyStyle(hwnd);
    updateShadow(hwnd);

    // ---- FINAL WHITE FLASH FIX ----

    // Maak tijdelijk layered en transparant
    LONG exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

    SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);

    // Forceer compositor cycle
    UpdateWindow(hwnd);

    // Verwijder layered na eerste event loop tick
    QTimer::singleShot(0, this, [hwnd]() {

        if (!IsWindow(hwnd))
            return;

        LONG exStyleNow = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE,
                         exStyleNow & ~WS_EX_LAYERED);

        SetWindowPos(hwnd, nullptr,
                     0,0,0,0,
                     SWP_NOMOVE | SWP_NOSIZE |
                         SWP_NOZORDER | SWP_FRAMECHANGED);
    });

    connect(window, &QObject::destroyed, this, [this, hwnd]() {
        mWindows.remove(hwnd);
    });

#endif
}

void WindowsCaptionHelper::detachFrom(QWindow *window)
{
    if (!window)
        return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    mWindows.remove(hwnd);
}

void WindowsCaptionHelper::setResizable(QWindow *window, bool value)
{
    if (!window)
        return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!mWindows.contains(hwnd))
        return;

    mWindows[hwnd].userResizable = value;
    applyStyle(hwnd);
}

void WindowsCaptionHelper::applyStyle(HWND hwnd)
{
    if (!mWindows.contains(hwnd))
        return;

    auto data = mWindows.value(hwnd);
    QWindow *window = data.window.data();
    if (!window)
        return;

    bool maximized = IsZoomed(hwnd);

    bool effectiveResizable =
        data.userResizable &&
        !isFixedSize(window) &&
        !maximized;

    LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);

    style &= ~WS_CAPTION;
    style |= WS_MINIMIZEBOX;

    if (effectiveResizable) {
        style |= WS_THICKFRAME;
        style |= WS_MAXIMIZEBOX;
    } else {
        style &= ~WS_THICKFRAME;
        style &= ~WS_MAXIMIZEBOX;
    }

    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    SetWindowPos(hwnd, nullptr,
                 0,0,0,0,
                 SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOZORDER | SWP_FRAMECHANGED);
}

void WindowsCaptionHelper::updateShadow(HWND hwnd)
{
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

#endif

bool WindowsCaptionHelper::nativeEventFilter(
    const QByteArray &eventType,
    void *message,
    qintptr *result)
{
#ifdef Q_OS_WINDOWS
    if (eventType != "windows_generic_MSG")
        return false;

    MSG *msg = static_cast<MSG *>(message);

    if (!mWindows.contains(msg->hwnd))
        return false;

    switch (msg->message)
    {
    case WM_SIZE:
        applyStyle(msg->hwnd);
        break;

    case WM_NCHITTEST:
    {
        auto data = mWindows.value(msg->hwnd);
        QWindow *window = data.window.data();
        if (!window)
            return false;

        bool maximized = IsZoomed(msg->hwnd);
        bool resizable =
            data.userResizable &&
            !isFixedSize(window) &&
            !maximized;

        POINT pt;
        pt.x = GET_X_LPARAM(msg->lParam);
        pt.y = GET_Y_LPARAM(msg->lParam);
        ScreenToClient(msg->hwnd, &pt);

        RECT rect;
        GetClientRect(msg->hwnd, &rect);

        const int border = 8;

        if (resizable)
        {
            if (pt.y < border) {
                if (pt.x < border) { *result = HTTOPLEFT; return true; }
                if (pt.x > rect.right - border) { *result = HTTOPRIGHT; return true; }
                *result = HTTOP; return true;
            }

            if (pt.y > rect.bottom - border) {
                if (pt.x < border) { *result = HTBOTTOMLEFT; return true; }
                if (pt.x > rect.right - border) { *result = HTBOTTOMRIGHT; return true; }
                *result = HTBOTTOM; return true;
            }

            if (pt.x < border) { *result = HTLEFT; return true; }
            if (pt.x > rect.right - border) { *result = HTRIGHT; return true; }
        }

        if (pt.y < mCaptionHeight) {
            *result = HTCAPTION;
            return true;
        }

        break;
    }
    case WM_WINDOWPOSCHANGED:
    {
        if (!mWindows.contains(msg->hwnd))
            break;

        QWindow* window = mWindows[msg->hwnd].window.data();
        if (!window)
            break;

        bool isFullscreen = (window->visibility() == QWindow::FullScreen);
        bool isMaximized  = IsZoomed(msg->hwnd);

        int corner;

        if (isFullscreen || isMaximized)
            corner = DWMWCP_DONOTROUND;
        else
            corner = DWMWCP_ROUND;

        DwmSetWindowAttribute(
            msg->hwnd,
            DWMWA_WINDOW_CORNER_PREFERENCE,
            &corner,
            sizeof(corner));

        break;
    }
    }

#endif
    return false;
}

void WindowsCaptionHelper::maximize(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd)
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#else
    window->showMaximized();
#endif
}

void WindowsCaptionHelper::restore(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd)
        SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
#else
    window->showNormal();
#endif
}

void WindowsCaptionHelper::toggleMaximize(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return;

    if (IsZoomed(hwnd))
        SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    else
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#else
    if (window->visibility() == QWindow::Maximized)
        window->showNormal();
    else
        window->showMaximized();
#endif
}

void WindowsCaptionHelper::showFullScreen(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return;

    window->showFullScreen();

    LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~WS_THICKFRAME;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    SetWindowPos(hwnd, nullptr,
                 0,0,0,0,
                 SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOZORDER | SWP_FRAMECHANGED);
#else
    window->showFullScreen();
#endif
}

void WindowsCaptionHelper::toggleFullScreen(QWindow *window)
{
    if (!window)
        return;

#ifdef Q_OS_WINDOWS
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return;

    if (window->visibility() == QWindow::FullScreen)
    {
        // Leaving fullscreen
        window->showNormal();

        applyStyle(hwnd);
    }
    else
    {
        // Enter fullscreen
        window->showFullScreen();

        LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~WS_THICKFRAME;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);

        SetWindowPos(hwnd, nullptr,
                     0,0,0,0,
                     SWP_NOMOVE | SWP_NOSIZE |
                         SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#else
    if (window->visibility() == QWindow::FullScreen)
        window->showNormal();
    else
        window->showFullScreen();
#endif
}
