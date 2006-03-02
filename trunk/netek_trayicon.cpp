#include "netek_trayicon.h"

// TODO 1.0: transparent X11 icon
// TODO 1.0: fix gnome systray icon
// TODO 1.0: include .ico as resource under windows

#ifdef Q_OS_UNIX
#include <QX11Info>
#include <X11/Xlib.h>
#define SYSTEM_TRAY_REQUEST_DOCK    0
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

namespace neteK {

#ifdef Q_OS_UNIX
class TrapErrorsX11 {
	static bool error, already;
	static int (*handler)(Display *, XErrorEvent *);

	static int myhandler(Display *, XErrorEvent *)
	{
		error = true;
		return 0;
	}
public:
	TrapErrorsX11()
	{
		Q_ASSERT(!already);

		already = true;
		error = false;
		handler = XSetErrorHandler(myhandler);
	}

	operator bool () const { return error; }

	~TrapErrorsX11()
	{
		XSetErrorHandler(handler);
		already = false;
	}
};

bool TrapErrorsX11::error;
bool TrapErrorsX11::already = false;
int (*TrapErrorsX11::handler)(Display *, XErrorEvent *) = 0;

class TrayIconX11: public QWidget {
	Q_OBJECT;

	QPointer<QMainWindow> m_owner;
	QPixmap m_icon;

signals:
	void activated();
	void showMenu(const QPoint &pos);

public:
	TrayIconX11(QMainWindow *owner)
	: m_owner(owner), m_icon(QPixmap(":/icons/netek.png"))
	{
		setWindowTitle(qApp->applicationName());
		recheck();
	}

	bool event(QEvent *e)
	{
		if(e->type() == QEvent::ToolTip) {
			if(m_owner)
				QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(), m_owner->windowTitle(), this);
		} else
			return QWidget::event(e);

		return true;
	}

	void closeEvent(QCloseEvent *e)
	{
		hide();
		scheduleRecheck();
		e->ignore();
	}

	void paintEvent(QPaintEvent *)
	{
		QPainter p(this);
		p.setRenderHint(QPainter::SmoothPixmapTransform);
		p.drawPixmap(rect(), m_icon);
	}

	void mouseReleaseEvent(QMouseEvent *e)
	{
		if(e->button() == Qt::LeftButton)
			emit activated();
		else if(e->button() == Qt::RightButton)
			emit showMenu(e->globalPos());

		QWidget::mouseReleaseEvent(e);
	}

public slots:
	void scheduleRecheck()
	{
		//qWarning("Trying to create tray icon in 3 seconds...");
		QTimer::singleShot(3000, this, SLOT(recheck()));
	}

	void recheck()
	{
		{
			TrapErrorsX11 err;

			Atom a = XInternAtom(x11Info().display(), QString("_NET_SYSTEM_TRAY_S%1").arg(x11Info().appScreen()).toUtf8().data(), True);
			//qDebug() << "Atom:" << a;
			if(a == None) {
				scheduleRecheck();
				return;
			}

			Window w = XGetSelectionOwner(x11Info().display(), a);
			//qDebug() << "Window:" << w;
			if(w == None) {
				scheduleRecheck();
				return;
			}

			{
				XEvent ev;

				memset(&ev, 0, sizeof(ev));
				ev.xclient.type = ClientMessage;
				ev.xclient.window = w;
				ev.xclient.message_type = XInternAtom (x11Info().display(), "_NET_SYSTEM_TRAY_OPCODE", False);
				ev.xclient.format = 32;
				ev.xclient.data.l[0] = x11Info().appTime();
				ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
				ev.xclient.data.l[2] = winId();

				XSendEvent(x11Info().display(), w, False, NoEventMask, &ev);
				XSync(x11Info().display(), False);
			}

			if(err) {
				scheduleRecheck();
				return;
			}
		}

		show();
	}
};
#endif

#ifdef Q_OS_WIN32
class TrayIconWin32: public QWidget {
    Q_OBJECT;

    HANDLE m_hIcon;
    bool m_notify;

    bool notify(DWORD msg)
    {
        NOTIFYICONDATA ndata;
        ZeroMemory(&ndata, sizeof(ndata));
        ndata.cbSize = sizeof(ndata);
        ndata.hWnd = winId();
        ndata.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        ndata.uCallbackMessage = WM_USER;
        wcscpy(ndata.szTip, qApp->applicationName().toStdWString().substr(0, 50).c_str());
        if(m_hIcon)
            ndata.hIcon = (HICON)m_hIcon;

        return Shell_NotifyIcon(msg, &ndata);
    }

    TrayIconWin32(bool &ok)
    : m_notify(false)
    {
        setAttribute(Qt::WA_DeleteOnClose);

        ok = (m_hIcon = LoadImage(0, L"netek.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE))
            && (m_notify = notify(NIM_ADD));

        //QTimer::singleShot(1000, this, SLOT(close()));
    }

public:
    static TrayIconWin32 *make()
    {
        bool ok;
        QPointer<TrayIconWin32> ticon = new TrayIconWin32(ok);
        if(!ok)
            delete ticon;

        return ticon;
    }

    ~TrayIconWin32()
    {
        if(m_notify)
            notify(NIM_DELETE);

        if(m_hIcon)
            CloseHandle(m_hIcon);
    }

    bool winEvent(MSG *message, long *result)
    {
        if(message->message == WM_USER) {
            if(message->lParam == WM_LBUTTONUP)
                emit activated();
            else if(message->lParam == WM_RBUTTONUP || message->lParam == WM_CONTEXTMENU)
                emit showMenu(QCursor::pos());
        } else
            return QWidget::winEvent(message, result);

        return true;
    }

signals:
	void activated();
	void showMenu(const QPoint &pos);
};
#endif

}

QObject *neteK::makeTrayIcon(QMainWindow *owner)
{
#if defined(Q_OS_UNIX)
	return new TrayIconX11(owner);
#elif defined(Q_OS_WIN32)
    return TrayIconWin32::make();
#else
	return 0;
#endif
}

#include "netek_trayicon.moc"
