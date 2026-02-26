#include "windowscaptionhelper.h"

#include <QGuiApplication>

#ifdef Q_OS_WINDOWS
#include <windowsx.h>
#endif

#ifdef Q_OS_WINDOWS
WindowsCaptionHelper* WindowsCaptionHelper::instance = nullptr;
#endif


WindowsCaptionHelper::WindowsCaptionHelper( QObject *parent )
    :   QObject { parent },
        mCaptionHeight { 28 },
        mWindow {}
{
#ifdef Q_OS_WINDOWS
    instance = this;
#endif
}


int WindowsCaptionHelper::getCaptionHeight() const {
    return mCaptionHeight;
}


void WindowsCaptionHelper::setCaptionHeight( const int &height ) {
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

    mWindow = window;
    mHwnd = reinterpret_cast<HWND>( window->winId() );

    // Store original window procedure
    mOriginalWindowProc = reinterpret_cast<WNDPROC> (
        SetWindowLongPtr(
            mHwnd,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>( WndProc )
        )
    );
#endif
}


#ifdef Q_OS_WINDOWS
LRESULT CALLBACK WindowsCaptionHelper::WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
) {
    if ( msg == WM_NCHITTEST && instance ) {
        POINT pt;
        pt.x = GET_X_LPARAM( lParam );
        pt.y = GET_Y_LPARAM( lParam );

        ScreenToClient( hwnd, &pt );

        if ( pt.y < instance->mCaptionHeight ) {
            return HTCAPTION;
        }
    }

    // Call original WindowProc safely
    if ( instance && instance->mOriginalWindowProc ) {
        return CallWindowProc(
            instance->mOriginalWindowProc,
            hwnd,
            msg,
            wParam,
            lParam
        );
    }

    // Fallback
    return DefWindowProc( hwnd, msg, wParam, lParam );
}
#endif
