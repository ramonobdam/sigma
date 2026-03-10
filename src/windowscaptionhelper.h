// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef WINDOWSCAPTIONHELPER_H
#define WINDOWSCAPTIONHELPER_H

#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QtTypes>
#include <QWindow>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

// This is a helper class that provides native Windows caption behavior for
// frameless Qt/QML windows. It implements custom hit-testing for resize and
// drag, manages exclusion zones, synchronizes window styles (WS_THICKFRAME,
// WS_MAXIMIZEBOX), handles DPI changes and keeps DWM features such as
// shadows, rounded corners, and Snap Layout support working correctly on
// Windows 10 and 11.
class WindowsCaptionHelper : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

    Q_PROPERTY(
        int captionHeight
        READ getCaptionHeight
        WRITE setCaptionHeight
        NOTIFY captionHeightChanged
    )

public:
    WindowsCaptionHelper( QObject *parent = nullptr );
    ~WindowsCaptionHelper();

    bool nativeEventFilter(
        const QByteArray &eventType,
        void *message,
        qintptr *result
    ) override;
    int getCaptionHeight() const;
    void setCaptionHeight( const int &height );

    Q_INVOKABLE void attachTo( QWindow *window );
    Q_INVOKABLE void detachFrom( QWindow *window );
    Q_INVOKABLE void maximize( QWindow *window );
    Q_INVOKABLE void restore( QWindow *window );
    Q_INVOKABLE void setExclusionRects(
        QWindow *window,
        const QList<QRect> &rects
    );
    Q_INVOKABLE void setResizable( QWindow *window, const bool &value );
    Q_INVOKABLE void showFullScreen( QWindow *window );
    Q_INVOKABLE void toggleFullScreen( QWindow *window );
    Q_INVOKABLE void toggleMaximize( QWindow *window );

signals:
    void captionHeightChanged();

private:
#ifdef Q_OS_WINDOWS
    void applyStyle( HWND hwnd );
    void updateShadow( HWND hwnd );
    static bool isFixedSize( QWindow *window );
    static constexpr double defaultDPI { 96. };     // 96 DPI = scale 1.0
    static double dpiScale( HWND hwnd );

    struct WindowData {
        QPointer<QWindow> window;
        bool userResizable = true;
        QList<QRect> exclusions;
    };

    QHash<HWND, WindowData> mWindows;
#endif

    int mCaptionHeight;
};

#endif
