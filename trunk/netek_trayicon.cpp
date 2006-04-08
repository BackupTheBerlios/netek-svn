// This file is part of neteK project http://netek.berlios.de
// Copyright (C) 2005-2006 Egon Kocjan <egon.kocjan@xlab.si>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "netek_trayicon.h"

// TODO: transparent X11 icon
// TODO: blinking indicator

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#define SYSTEM_TRAY_REQUEST_DOCK    0
#define XEMBED_EMBEDDED_NOTIFY		0
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#include "netek_rc.h"
#endif

namespace neteK {

#ifdef Q_WS_X11
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

class TrayIconX11: public TrayIcon {
	Q_OBJECT;

	QPointer<QMainWindow> m_owner;
	QPixmap m_icon_active, m_icon_inactive;
	bool m_active;
	bool m_docked;
	//Atom m_xembed;

public:
	TrayIconX11(QMainWindow *owner)
	: m_owner(owner), m_icon_active(QPixmap(":/icons/netek.png")),
		m_icon_inactive(QPixmap(":/icons/netek_grey.png")),
		m_active(false), m_docked(false)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle(qApp->applicationName());
		setMinimumSize(24,24); // a hack for GNOME...

		/*{
			TrapErrorsX11 err;
			m_xembed = XInternAtom(x11Info().display(), "_XEMBED", True);
		}*/

		recheck();
	}

	bool event(QEvent *e)
	{
		if(e->type() == QEvent::ToolTip) {
			if(m_owner)
				QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(), m_owner->windowTitle(), this);
		}
		
		return QWidget::event(e);
	}

	void paintEvent(QPaintEvent *)
	{
		QPainter p(this);
		p.setRenderHint(QPainter::SmoothPixmapTransform);
		p.drawPixmap(rect(), m_active ? m_icon_active : m_icon_inactive);
	}

	void mouseReleaseEvent(QMouseEvent *e)
	{
		if(e->button() == Qt::LeftButton)
			emit activated();
		else if(e->button() == Qt::RightButton)
			emit showMenu(e->globalPos());

		QWidget::mouseReleaseEvent(e);
	}

	bool x11Event(XEvent *e)
	{
		// does not always work under GNOME..?
		/*if(e->type == ClientMessage && e->xclient.message_type == m_xembed) {
			if(e->xclient.data.l[1] == XEMBED_EMBEDDED_NOTIFY) {
				m_docked = true;
				show();
			}
		}*/

		if(e->type == ReparentNotify) {
			m_docked = true;
			show();
		}

		return QWidget::x11Event(e);
	}
	
	void setActive(bool yes)
	{
		m_active = yes;
		update();
	}

public slots:
	void scheduleRecheck()
	{
		// this is really ugly, yes, if someone has a proper pure qt4
		// implementation of fd.o tray icon, I will put it in
		QTimer::singleShot(1000, this, SLOT(recheck()));
	}

	void recheck()
	{
		if(m_docked)
			return;

		scheduleRecheck();

		TrapErrorsX11 err;

		Atom a = XInternAtom(x11Info().display(), QString("_NET_SYSTEM_TRAY_S%1").arg(x11Info().appScreen()).toUtf8().data(), True);
		//qDebug() << "Atom:" << a;
		if(a == None)
			return;

		Window w = XGetSelectionOwner(x11Info().display(), a);
		//qDebug() << "Window:" << w;
		if(w == None)
			return;

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
	}


};
#endif

#ifdef Q_WS_WIN
class TrayIconWin32: public TrayIcon {
	Q_OBJECT;
	
	HANDLE m_icon_active, m_icon_inactive;
	bool m_active, m_notify;
	
	bool notify(DWORD msg)
	{
		NOTIFYICONDATA ndata;
		ZeroMemory(&ndata, sizeof(ndata));
		ndata.cbSize = sizeof(ndata);
		ndata.hWnd = winId();
		ndata.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		ndata.uCallbackMessage = WM_USER;
		wcscpy(ndata.szTip, qApp->applicationName().toStdWString().substr(0, 50).c_str());
		ndata.hIcon = (HICON)(m_active ? m_icon_active : m_icon_inactive);
		
		return Shell_NotifyIcon(msg, &ndata);
	}

	TrayIconWin32(bool &ok)
	: m_active(false), m_notify(false)
	{
		setAttribute(Qt::WA_DeleteOnClose);

		ok = (m_icon_active = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(NETEK_ICON_ACTIVE)))
			&& (m_icon_inactive = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(NETEK_ICON_INACTIVE)))
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
	
		if(m_icon_active)
			CloseHandle(m_icon_active);
			
		if(m_icon_inactive)
			CloseHandle(m_icon_inactive);
	}

	bool winEvent(MSG *message, long *result)
	{
		if(message->message == WM_USER) {
			if(message->lParam == WM_LBUTTONUP)
				emit activated();
			else if(message->lParam == WM_RBUTTONUP || message->lParam == WM_CONTEXTMENU)
				emit showMenu(QCursor::pos());
		}
		
		return QWidget::winEvent(message, result);
	}
	
	void setActive(bool yes)
	{
		m_active = yes;
		notify(NIM_MODIFY);
	}
};
#endif

}

neteK::TrayIcon *neteK::TrayIcon::make(QMainWindow *owner)
{
#if defined(Q_WS_X11)
	return new TrayIconX11(owner);
#elif defined(Q_WS_WIN)
	return TrayIconWin32::make();
#else
	return 0;
#endif
}

#include "netek_trayicon.moc"
