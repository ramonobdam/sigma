// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "windowscaptionhelper.h"
#include <QGuiApplication>
#include <QTimer>

#ifdef Q_OS_WINDOWS
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif


WindowsCaptionHelper::WindowsCaptionHelper( QObject *parent )
    :   QObject { parent },
        mCaptionHeight { 32 }
{
#ifdef Q_OS_WINDOWS
    qApp->installNativeEventFilter( this );
#endif
}


WindowsCaptionHelper::~WindowsCaptionHelper() {
#ifdef Q_OS_WINDOWS
    qApp->removeNativeEventFilter( this );
#endif
}


bool WindowsCaptionHelper::nativeEventFilter(
    const QByteArray &eventType,
    void *message,
    qintptr *result
) {
#ifdef Q_OS_WINDOWS
    if ( eventType != "windows_generic_MSG" ) {
        return false;
    }

    MSG *msg { static_cast<MSG*>( message ) };
    HWND hwnd { msg->hwnd };
    if ( !mWindows.contains( hwnd ) ) {
        return false;
    }

    const WindowData &data { mWindows[ hwnd ] };
    QWindow *window { data.window.data() };
    if ( !window ) {
        return false;
    }

    bool effectiveResizable {
        data.userResizable &&
        !isFixedSize( window ) &&
        !IsZoomed( hwnd )
    };

    switch ( msg->message ) {
        // Resize style sync (maximized state)
        case WM_SIZE:
            applyStyle( hwnd );
            return false;

        // Hit testing (drag + resize + exclusions)
        case WM_NCHITTEST:
        {
            // Early exit: fullscreen
            if ( window->visibility() == QWindow::FullScreen ) {
                *result = HTCLIENT;
                return true;
            }

            // Cursor in physical px
            POINT ptPx;
            ptPx.x = GET_X_LPARAM( msg->lParam );
            ptPx.y = GET_Y_LPARAM( msg->lParam );
            ScreenToClient( hwnd, &ptPx );

            // DPI to DIPs
            const double scale { dpiScale( hwnd ) };
            const int x { int( ptPx.x / scale ) };
            const int y { int( ptPx.y / scale ) };

            RECT rectPx;
            GetClientRect( hwnd, &rectPx );

            const int width  { int( rectPx.right  / scale ) };
            const int height { int( rectPx.bottom / scale ) };

            // Resize border thickness (DIP)
            const int border {
                static_cast<int> (
                    (
                        GetSystemMetrics(SM_CXSIZEFRAME) +
                        GetSystemMetrics(SM_CXPADDEDBORDER)
                    )
                    / scale
                )
            };

            // Resize action
            if ( effectiveResizable ) {
                const bool left { x < border };
                const bool right { x > width  - border };
                const bool top { y < border };
                const bool bottom { y > height - border };

                // Check corners
                if ( top && left ) {
                    *result = HTTOPLEFT;
                    return true;
                }
                if ( top && right ) {
                    *result = HTTOPRIGHT;
                    return true;
                }
                if ( bottom && left ) {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                if ( bottom && right ) {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }

                // Check sides
                if ( top ) {
                    *result = HTTOP;
                    return true;
                }
                if ( bottom ) {
                    *result = HTBOTTOM;
                    return true;
                }
                if ( left ) {
                    *result = HTLEFT;
                    return true;
                }
                if ( right ) {
                    *result = HTRIGHT;
                    return true;
                }
            }

            // Exclusions
            const QList<QRect> exclusions { data.exclusions };
            for ( const QRect &r : exclusions ) {
                if ( r.contains(x, y) ) {
                    return false;
                }
            }

            // Drag zone
            if ( y < mCaptionHeight ) {
                *result = HTCAPTION;
                return true;
            }

            return false;
        }

        // Double click to maximize
        case WM_NCLBUTTONDBLCLK:
        {
            bool canResize = data.userResizable && !isFixedSize( window );
            if ( !canResize ) {
                return false;
            }

            if ( IsZoomed( hwnd ) ) {
                SendMessage( hwnd, WM_SYSCOMMAND, SC_RESTORE, 0 );
            }
            else {
                SendMessage( hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
            }

            return true;
        }

        // Windows 11 corner sync
        case WM_WINDOWPOSCHANGED:
        {
            bool isFullscreen { window->visibility() == QWindow::FullScreen };

            bool isMaximized { static_cast<bool>( IsZoomed( hwnd ) ) };

            int corner { DWMWCP_ROUND };

            if ( isFullscreen || isMaximized ) {
                corner = DWMWCP_DONOTROUND;
            }

            DwmSetWindowAttribute(
                hwnd,
                DWMWA_WINDOW_CORNER_PREFERENCE,
                &corner,
                sizeof( corner )
            );

            return false;
        }

        case WM_DPICHANGED:
        {
            applyStyle( hwnd );
            updateShadow( hwnd );

            return false;
        }
    }
#endif
    return false;
}


int WindowsCaptionHelper::getCaptionHeight() const {
    return mCaptionHeight;
}


void WindowsCaptionHelper::setCaptionHeight( int height ) {
    if ( mCaptionHeight == height ) {
        return;
    }

    mCaptionHeight = height;
    emit captionHeightChanged();
}


void WindowsCaptionHelper::attachTo( QWindow *window ) {
#ifdef Q_OS_WINDOWS
    if ( !window ) {
        return;
    }

    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !hwnd || mWindows.contains( hwnd ) ) {
        return;
    }

    WindowData data {};
    data.window = window;
    mWindows.insert( hwnd, data );

    applyStyle( hwnd );
    updateShadow( hwnd );

    // White flash fix

    // Set temporarily to layered and transparant
    LONG exStyle = GetWindowLongPtr( hwnd, GWL_EXSTYLE );
    SetWindowLongPtr( hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED );

    SetLayeredWindowAttributes( hwnd, 0, 0, LWA_ALPHA );

    // Force compositor cycle
    UpdateWindow(hwnd);

    // Removed layered after first event loop tick
    QTimer::singleShot( 0, this, [ hwnd ]() {
        if ( !IsWindow( hwnd ) ) {
            return;
        }

        LONG exStyleNow {
            static_cast<LONG>( GetWindowLongPtr( hwnd, GWL_EXSTYLE ) )
        };
        SetWindowLongPtr( hwnd, GWL_EXSTYLE, exStyleNow & ~WS_EX_LAYERED );

        SetWindowPos(
            hwnd,
            nullptr,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );
    });

    // Remove window from the QHash when it is destroyed
    connect (
        window,
        &QObject::destroyed,
        this,
        [ this, hwnd ]() { mWindows.remove( hwnd ); }
    );

#endif
}


void WindowsCaptionHelper::detachFrom( QWindow *window ) {
#ifdef Q_OS_WINDOWS
    if ( !window ) {
        return;
    }

    // Remove window from the QHash
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    mWindows.remove( hwnd );
#endif
}


void WindowsCaptionHelper::maximize( QWindow *window ) {
    // On Windows, send system command to maximize the window. Otherwise, the
    // Qt command is used.
    if ( !window ) {
        return;
    }

#ifdef Q_OS_WINDOWS
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( hwnd ) {
        SendMessage( hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
    }
#else
    window->showMaximized();
#endif
}


void WindowsCaptionHelper::restore( QWindow *window ) {
    // On Windows, send system command to restore the window. Otherwise, the
    // Qt command is used.
    if ( !window ) {
        return;
    }

#ifdef Q_OS_WINDOWS
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( hwnd ) {
        SendMessage( hwnd, WM_SYSCOMMAND, SC_RESTORE, 0 );
    }
#else
    window->showNormal();
#endif
}


void WindowsCaptionHelper::setExclusionRects(
    QWindow *window,
    const QList<QRect> &rects
) {
// Set the exclusion rectangles where hit detection is disabled. Typically used
// for custom buttons or menus in the title bar.
#ifdef Q_OS_WINDOWS
    if ( !window ) {
        return;
    }
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !mWindows.contains( hwnd ) ) {
        return;
    }

    mWindows[ hwnd ].exclusions = rects;
#endif
}


void WindowsCaptionHelper::setResizable( QWindow *window, bool value ) {
#ifdef Q_OS_WINDOWS
    if ( !window ) {
        return;
    }

    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !mWindows.contains(hwnd) ) {
        return;
    }

    mWindows[ hwnd ].userResizable = value;
    applyStyle( hwnd );
#endif
}


void WindowsCaptionHelper::showFullScreen( QWindow *window ) {
    if ( !window ) {
        return;
    }

#ifdef Q_OS_WINDOWS
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !hwnd ) {
        return;
    }

    window->showFullScreen();

    // Remove resize border and shadow when fullscreen
    LONG style { static_cast<LONG>( GetWindowLongPtr( hwnd, GWL_STYLE ) )};
    style &= ~WS_THICKFRAME;
    SetWindowLongPtr( hwnd, GWL_STYLE, style );

    // Redraw window
    SetWindowPos(
        hwnd,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );
#else
    window->showFullScreen();
#endif
}


void WindowsCaptionHelper::toggleFullScreen( QWindow *window ) {
    if ( !window ) {
        return;
    }

#ifdef Q_OS_WINDOWS
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !hwnd ) {
        return;
    }

    if ( window->visibility() == QWindow::FullScreen ) {
        // Leaving fullscreen
        window->showNormal();
        applyStyle( hwnd );
    }
    else {
        // Enter fullscreen
        window->showFullScreen();

        // Remove resize border and shadow when fullscreen
        LONG style { static_cast<LONG>( GetWindowLongPtr( hwnd, GWL_STYLE ) ) };
        style &= ~WS_THICKFRAME;
        SetWindowLongPtr( hwnd, GWL_STYLE, style );

        // Redraw window
        SetWindowPos(
            hwnd,
            nullptr,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );
    }
#else
    if ( window->visibility() == QWindow::FullScreen ){
        window->showNormal();
    }
    else {
        window->showFullScreen();
    }
#endif
}


void WindowsCaptionHelper::toggleMaximize( QWindow *window ) {
    // On windows, send system commands to restore/maximize. Otherwise, use Qt
    // commands.
    if ( !window ) {
        return;
    }

#ifdef Q_OS_WINDOWS
    HWND hwnd { reinterpret_cast<HWND>( window->winId() ) };
    if ( !hwnd ) {
        return;
    }

    if ( IsZoomed( hwnd ) ) {
        SendMessage( hwnd, WM_SYSCOMMAND, SC_RESTORE, 0 );
    }
    else {
        SendMessage( hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
    }
#else
    if ( window->visibility() == QWindow::Maximized ) {
        window->showNormal();
    }
    else {
        window->showMaximized();
    }
#endif
}


#ifdef Q_OS_WINDOWS
void WindowsCaptionHelper::applyStyle( HWND hwnd ) {
    if ( !mWindows.contains( hwnd ) ) {
        return;
    }

    const WindowData &data { mWindows[ hwnd ] };
    QWindow *window { data.window.data() };
    if ( !window ) {
        return;
    }

    bool maximized { static_cast<bool>( IsZoomed( hwnd ) ) };

    bool effectiveResizable =
        data.userResizable &&
        !isFixedSize( window ) &&
        !maximized;

    LONG style { static_cast<LONG>( GetWindowLongPtr( hwnd, GWL_STYLE ) ) };

    style &= ~WS_CAPTION;
    style |= WS_MINIMIZEBOX;
    style |= WS_SYSMENU;

    if ( effectiveResizable ) {
        style |= WS_THICKFRAME;
        style |= WS_MAXIMIZEBOX;
    }
    else {
        style &= ~WS_THICKFRAME;
        style &= ~WS_MAXIMIZEBOX;
    }

    SetWindowLongPtr( hwnd, GWL_STYLE, style );

    SetWindowPos(
        hwnd,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );
}


void WindowsCaptionHelper::updateShadow( HWND hwnd ) {
    MARGINS margins { -1 };
    DwmExtendFrameIntoClientArea( hwnd, &margins );
}


bool WindowsCaptionHelper::isFixedSize( QWindow *window ) {
    if ( !window ) {
        return false;
    }

    return (
        window->minimumSize().isValid() &&
        window->maximumSize().isValid() &&
        window->minimumSize() == window->maximumSize()
    );
}


double WindowsCaptionHelper::dpiScale( HWND hwnd ) {
    return GetDpiForWindow( hwnd ) / sDefaultDPI;
}
#endif
