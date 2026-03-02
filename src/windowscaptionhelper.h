#ifndef WINDOWSCAPTIONHELPER_H
#define WINDOWSCAPTIONHELPER_H

#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QPointer>
#include <QWindow>
#include <QObject>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

class WindowsCaptionHelper :
                             public QObject,
                             public QAbstractNativeEventFilter
{
    Q_OBJECT

    Q_PROPERTY(int captionHeight
                   READ captionHeight
                       WRITE setCaptionHeight
                           NOTIFY captionHeightChanged)

public:
    explicit WindowsCaptionHelper(QObject *parent = nullptr);
    ~WindowsCaptionHelper() override;

    bool nativeEventFilter(
        const QByteArray &eventType,
        void *message,
        qintptr *result) override;

    int captionHeight() const;
    void setCaptionHeight(int height);

    Q_INVOKABLE void attachTo(QWindow *window);
    Q_INVOKABLE void detachFrom(QWindow *window);
    Q_INVOKABLE void setResizable(QWindow *window, bool value);

    Q_INVOKABLE void maximize(QWindow *window);
    Q_INVOKABLE void restore(QWindow *window);
    Q_INVOKABLE void toggleMaximize(QWindow *window);

    Q_INVOKABLE void showFullScreen(QWindow *window);
    Q_INVOKABLE void toggleFullScreen(QWindow *window);

signals:
    void captionHeightChanged();

private:
#ifdef Q_OS_WINDOWS
    struct WindowData {
        QPointer<QWindow> window;
        bool userResizable = true;
    };

    QHash<HWND, WindowData> mWindows;

    void applyStyle(HWND hwnd);
    void updateShadow(HWND hwnd);
    static bool isFixedSize(QWindow *window);
#endif

    int mCaptionHeight = 32;
};

#endif
