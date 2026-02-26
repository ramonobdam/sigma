#ifndef WINDOWSCAPTIONHELPER_H
#define WINDOWSCAPTIONHELPER_H

#include <QByteArray>
#include <QObject>
#include <QWindow>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

// WindowsCaptionHelper is a small Windows-specific utility class that enables
// fully native title bar dragging for frameless Qt windows. It intercepts the
// Win32 WM_NCHITTEST message and marks a configurable top region of the window
// as HTCAPTION, allowing the operating system to handle window movement
// natively.
class WindowsCaptionHelper : public QObject {
    Q_OBJECT
    Q_PROPERTY(
        int captionHeight
        READ getCaptionHeight
        WRITE setCaptionHeight
        NOTIFY captionHeightChanged
)

public:
    explicit WindowsCaptionHelper( QObject *parent = nullptr) ;

    int getCaptionHeight() const;
    void setCaptionHeight( const int &height );

    Q_INVOKABLE void attachTo( QWindow *window );

signals:
    void captionHeightChanged();

protected:
    bool nativeEventFilter(
        const QByteArray &eventType,
        void *message,
        qintptr *result
    );

private:
#ifdef Q_OS_WINDOWS
    static LRESULT CALLBACK WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );
    static WindowsCaptionHelper *instance;
    HWND mHwnd = nullptr;
#endif

    QWindow *mWindow;
    int mCaptionHeight;
};

#endif // WINDOWSCAPTIONHELPER_H
