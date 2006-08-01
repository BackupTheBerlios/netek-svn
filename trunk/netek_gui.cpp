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

#include "netek_gui.h"
#include "netek_share.h"
#include "netek_shares.h"
#include "netek_globalsettings.h"
#include "netek_trayicon.h"
#include "netek_settings.h"
#include "netek_netutils.h"
#include "netek_logviewer.h"
#include "netek_about.h"

namespace neteK {

class CopyLinkMenu: public QMenu {
	Q_OBJECT;

	QString m_protocol;
	quint16 m_port;
	QPointer<QAction> m_public;
	QIcon m_icon;

	QString link(QString addr)
	{
		return QString("%1://%2:%3")
			.arg(m_protocol)
			.arg(addr)
			.arg(m_port);
	}

	QString link(QHostAddress addr)
	{
		return link(addr.toString());
	}

	void setPublic(QString text)
	{
		if(m_public) {
			m_public->setIcon(m_icon);
			m_public->setText(text);

			QFont font = m_public->font();
			font.setItalic(false);
			m_public->setFont(font);

			m_public = 0;

			adjustSize();
		}
	}

public slots:
	void resolved(QHostAddress addr)
	{
		if(!m_public)
			return;

		if(addr.isNull()) {
			m_public->setText(tr("Autodetect failed"));
			adjustSize();
		} else
			setPublic(link(addr));
	}

	void copy(QAction *a)
	{
		if(a != m_public)
			qApp->clipboard()->setText(a->text());
	}

public:
	CopyLinkMenu(QString prot, quint16 port)
	: m_protocol(prot), m_port(port), m_icon(QPixmap(":/icons/www.png"))
	{
		connect(this, SIGNAL(triggered(QAction*)), SLOT(copy(QAction*)));

		setTitle(tr("Copy link"));
		setIcon(m_icon);

		m_public = addAction(tr("Autodetecting public address..."));
		{
			QFont font = m_public->font();
			font.setItalic(true);
			m_public->setFont(font);
		}

		QList<QPair<QString, QHostAddress> > nifs;
		if(networkInterfaces(nifs)) {
			QMap<int, QList<QHostAddress> > sorted;
			QList<QPair<QString, QHostAddress> >::iterator nif;
			for(nif = nifs.begin(); nif != nifs.end(); ++nif) {
				int idx = 3;

				if(nif->second.protocol() == QAbstractSocket::IPv4Protocol) {
					if(isLoopback(nif->second))
						continue;

					if(isPublicNetwork(nif->second))
						idx = 0;
					else if(isPrivateNetwork(nif->second))
						idx = 1;
					else
						idx = 2;

					sorted[idx].append(nif->second);
				}
			}

			foreach(QList<QHostAddress> alist, sorted)
				foreach(QHostAddress addr, alist)
					addAction(m_icon, link(addr));
		}

		Settings settings;
		if(settings.publicAddress() == Settings::PublicAddressManual)
			setPublic(link(settings.customPublicAddress()));
		else
			resolvePublicAddress(QHostAddress(), this, SLOT(resolved(QHostAddress)));
	}
};

}

neteK::Gui::Gui()
: m_save_geometry_timer(false)
{
	ui.setupUi(this);

	m_shares = new Shares;
	m_shares->setParent(this);
	
	m_tray_icon = TrayIcon::make(this);
	if(m_tray_icon) {
		connect(m_tray_icon, SIGNAL(activated()), SLOT(toggleVisible()));
		connect(m_tray_icon, SIGNAL(showMenu(const QPoint &)), SLOT(trayMenu(const QPoint &)));
		
		m_tray_icon_switcher = new TrayIconSwitcher(m_tray_icon);
		connect(m_shares, SIGNAL(transfer()), m_tray_icon_switcher, SLOT(transfer()));
	}

	ui.shareList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui.shareList, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(shareMenu()));

	connect(m_shares, SIGNAL(changed()), SLOT(sharesChanged()));

	connect(ui.shareList, SIGNAL(currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)), SLOT(sharesChanged()), Qt::QueuedConnection);
	connect(ui.shareList, SIGNAL(itemActivated(QTreeWidgetItem *, int)), SLOT(toggleRunStatus()));

	connect(ui.actionNetek, SIGNAL(triggered()), SLOT(toggleVisible()));
	connect(ui.action_Create_share, SIGNAL(triggered()), m_shares, SLOT(createShareWithSettings()));
	connect(ui.action_Create, SIGNAL(triggered()), m_shares, SLOT(createShareWithSettings()));
	connect(ui.actionSettings, SIGNAL(triggered()), SLOT(shareSettings()));
	connect(ui.actionStart, SIGNAL(triggered()), SLOT(startShare()));
	connect(ui.actionStop, SIGNAL(triggered()), SLOT(stopShare()));
	connect(ui.actionStop_all, SIGNAL(triggered()), SLOT(stopAllShares()));
	connect(ui.actionDelete, SIGNAL(triggered()), SLOT(deleteShare()));
	connect(ui.action_Global_settings, SIGNAL(triggered()), SLOT(globalSettings()));
	connect(ui.action_Quit, SIGNAL(triggered()), SLOT(quitRequest()));
	connect(ui.actionCopy_link, SIGNAL(triggered()), SLOT(copyLinkMenu()));
	connect(ui.actionShow_log, SIGNAL(triggered()), SLOT(showLog()));
	connect(ui.action_About, SIGNAL(triggered()), SLOT(showAbout()));

	connect(qApp, SIGNAL(command_quit()), SLOT(quitRequest()));
	connect(this, SIGNAL(userQuit()), qApp, SLOT(userQuit()), Qt::QueuedConnection);

	sharesChanged();

	{
		QRect geom = Settings().guiGeometry();
		if(geom.isValid()) {
			resize(geom.size());
			move(geom.topLeft());
		}
	}

	{
		QPointer<QHeaderView> h = ui.shareList->header();
		h->setStretchLastSection(false);

		QList<int> cwidth = Settings().guiShareListColumns();
		if(cwidth.size() == ui.shareList->columnCount())
			for(int i=0; i<cwidth.size(); ++i)
				if(cwidth.at(i) > 0)
					h->resizeSection(i, cwidth.at(i)); // TODO: breaks with qt 4.1.4

		connect(h, SIGNAL(sectionResized(int,int,int)), SLOT(saveGeometryTimer()));
	}

	if(!m_tray_icon)
		show();
}

void neteK::Gui::closeEvent(QCloseEvent *e)
{
	if(m_tray_icon) {
		hide();
		e->ignore();
	} else
		emit userQuit();
}

void neteK::Gui::resizeEvent(QResizeEvent *)
{ saveGeometryTimer(); }

void neteK::Gui::moveEvent(QMoveEvent *)
{ saveGeometryTimer(); }

void neteK::Gui::saveGeometryTimer()
{
	if(!m_save_geometry_timer) {
		m_save_geometry_timer = true;
		QTimer::singleShot(100, this, SLOT(saveGeometry()));
	}
}

void neteK::Gui::saveGeometry()
{
	Settings settings;
	m_save_geometry_timer = false;
	settings.setGuiGeometry(QRect(pos(), size()));

	QList<int> slc;
	for(int i=0; i<ui.shareList->columnCount(); ++i)
		slc.append(ui.shareList->columnWidth(i));

	settings.setGuiShareListColumns(slc);
}

void neteK::Gui::toggleVisible()
{
	if(isVisible())
		hide();
	else {
		show();
		activateWindow();
		raise();
	}
}

bool neteK::Gui::validAndConfigured(Share *sh)
{ return sh && sh->status() != Share::StatusUnconfigured; }

void neteK::Gui::makeMappedShareAction(Share *sh, QMenu *m, QSignalMapper *sm, int i, QAction *a)
{
	QPointer<QAction> new_a = m->addAction(a->icon(), a->text());
	sm->setMapping(new_a, i);
	if(a == ui.actionStart)
		new_a->setEnabled(!sh->runStatus());
	else if(a == ui.actionStop)
		new_a->setEnabled(sh->runStatus());

	connect(new_a, SIGNAL(triggered()), sm, SLOT(map()));
}

void neteK::Gui::trayMenu(const QPoint &pos)
{
	QMenu menu(qApp->applicationName());
	menu.addAction(ui.actionNetek);
	menu.setDefaultAction(ui.actionNetek);
	menu.addSeparator();
	menu.addAction(ui.action_Create_share);

	QPointer<QMenu> smenu = new QMenu(tr("&Shares"), &menu);
	smenu->setIcon(QPixmap(":/icons/folder_open.png"));
	menu.addMenu(smenu);

	{
		QList<QPointer<QSignalMapper> > mappers;
		for(int i=0; i<5; ++i)
			mappers.append(new QSignalMapper(smenu));

		connect(mappers.at(0), SIGNAL(mapped(int)), SLOT(copyLinkMenu(int)));
		connect(mappers.at(1), SIGNAL(mapped(int)), SLOT(shareSettings(int)));
		connect(mappers.at(2), SIGNAL(mapped(int)), SLOT(startShare(int)));
		connect(mappers.at(3), SIGNAL(mapped(int)), SLOT(stopShare(int)));
		connect(mappers.at(4), SIGNAL(mapped(int)), SLOT(deleteShare(int)));

		for(int i=0; i<m_shares->shares(); ++i) {
			QPointer<Share> sh = m_shares->share(i);
			if(validAndConfigured(sh)) {
				QPointer<QMenu> shmenu = new QMenu(sh->niceId(), smenu);
				shmenu->setIcon(shareIcon(sh));
				smenu->addMenu(shmenu);

				makeMappedShareAction(sh, shmenu, mappers.at(0), i, ui.actionCopy_link);
				makeMappedShareAction(sh, shmenu, mappers.at(1), i, ui.actionSettings);
				makeMappedShareAction(sh, shmenu, mappers.at(2), i, ui.actionStart);
				makeMappedShareAction(sh, shmenu, mappers.at(3), i, ui.actionStop);
				makeMappedShareAction(sh, shmenu, mappers.at(4), i, ui.actionDelete);
			}
		}
	}
	
	menu.addAction(ui.actionStop_all);
	menu.addSeparator();
	menu.addAction(ui.actionShow_log);
	menu.addAction(ui.action_Global_settings);
	menu.addSeparator();
	menu.addAction(ui.action_Quit);

	menu.exec(pos);
}

void neteK::Gui::shareMenu()
{
	QPointer<Share> sh = getShare();
	if(validAndConfigured(sh)) {
		QMenu menu;
		menu.addAction(ui.actionCopy_link);
		menu.addAction(ui.actionSettings);
		menu.addAction(ui.actionStart);
		menu.addAction(ui.actionStop);
		menu.addAction(ui.actionStop_all);
		menu.addAction(ui.actionDelete);

		menu.exec(QCursor::pos());
	}
}

neteK::Share *neteK::Gui::getShare(int id)
{
	return m_shares->share(
		id >= 0
			? id
			: ui.shareList->indexOfTopLevelItem(ui.shareList->currentItem()));
}

void neteK::Gui::shareSettings(int id)
{
	QPointer<Share> sh = getShare(id);
	if(sh)
		sh->showSettings();
}

void neteK::Gui::deleteShare(int id)
{
	m_shares->deleteShareWithQuestion(
		id >= 0
			? id
			: ui.shareList->indexOfTopLevelItem(ui.shareList->currentItem()));
}

void neteK::Gui::startShare(int id)
{
	QPointer<Share> sh = getShare(id);
	if(sh)
		sh->startIfStopped();
}

void neteK::Gui::stopShare(int id)
{
	QPointer<Share> sh = getShare(id);
	if(sh)
		sh->stop();
}

void neteK::Gui::stopAllShares()
{
	for(int i=0; i<m_shares->shares(); ++i)
		m_shares->share(i)->stop();
}

void neteK::Gui::toggleRunStatus()
{
	QPointer<Share> sh = getShare();
	if(sh) {
		if(sh->runStatus())
			sh->stop();
		else
			sh->startIfStopped();
	}
}

QPixmap neteK::Gui::shareIcon(Share *sh)
{
	switch(sh->status()) {
		case Share::StatusStarted:
			return QPixmap(":/icons/folder_open.png");
		case Share::StatusStopped:
			return QPixmap(":/icons/folder_grey.png");
		default:
			return QPixmap(":/icons/exec.png");
	}
}

bool neteK::Gui::getDragAndDropPath(const QMimeData *mime, QString &path)
{
	qDebug() << "DnD MIME:" << mime->formats();
	qDebug() << "DnD text:" << mime->text();
	qDebug() << "DnD URLs:" << mime->urls();
	
	QList<QString> files;

	foreach(QUrl url, mime->urls())
		if(Shares::resolveLocalPath(url.toLocalFile(), path))
			return true;

	return Shares::resolveAnyPath(mime->text(), path);
}

void neteK::Gui::dragEnterEvent(QDragEnterEvent *e)
{
	QString path;
	if(getDragAndDropPath(e->mimeData(), path)) {
		e->setDropAction(Qt::LinkAction);
		e->acceptProposedAction();
	}
}

void neteK::Gui::dropEvent(QDropEvent *e)
{
	QString path;
	if(getDragAndDropPath(e->mimeData(), path))
		m_shares->createShareWithSettings(path);
}

void neteK::Gui::sharesChanged()
{
	while(ui.shareList->topLevelItemCount() > m_shares->shares())
		delete ui.shareList->takeTopLevelItem(ui.shareList->topLevelItemCount()-1);

	bool is_active = false;
	
	for(int i=0; i<m_shares->shares(); ++i) {
		QPointer<Share> sh = m_shares->share(i);
		if(sh) {
			QTreeWidgetItem *item;
			if(i < ui.shareList->topLevelItemCount())
				item = ui.shareList->topLevelItem(i);
			else {
				item = new QTreeWidgetItem;
				item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
				for(int col=1; col<ui.shareList->columnCount(); ++col)
					item->setTextAlignment(col, Qt::AlignCenter);

				{
					QFont font = item->font(0);
					font.setBold(true);
					for(int col=0; col<ui.shareList->columnCount(); ++col)
						item->setFont(col, font);
				}

				ui.shareList->addTopLevelItem(item);
			}

			item->setText(0, sh->folder());
			item->setText(1, QString::number(sh->port()));
			item->setText(2, Share::niceType(sh->type()));

			{
				QString txt = Share::nicePermission(sh->permission());
				if(sh->access() == Share::AccessUsernamePassword)
					txt = QString("%1 (password required)").arg(txt);
				
				item->setText(3, txt);
			}

			QString status;
			QColor status_color;
			switch(sh->status()) {
				case Share::StatusStarted:
					status = tr("started");
					status_color = QColor(0, 0x99, 0);
					is_active = true;
					break;
				case Share::StatusStopped:
					status = tr("stopped");
					status_color = QColor(0x66, 0x66, 0x66);
					break;
				default:
					status = tr("processing");
					status_color = QColor(0xff, 0x66, 0);
			}

			item->setIcon(0, shareIcon(sh));
			item->setText(4, status);
			item->setTextColor(4, status_color);
			item->setText(5, QString::number(sh->clients()));
		}
	}
	
	if(m_tray_icon_switcher)
		m_tray_icon_switcher->setActive(is_active);

	ui.actionStop_all->setEnabled(is_active);
	
	{
		QPointer<Share> sh = getShare();
		bool ok = validAndConfigured(sh);
		ui.actionDelete->setEnabled(ok);
		ui.actionSettings->setEnabled(ok);
		ui.actionStart->setEnabled(ok && !sh->runStatus());
		ui.actionStop->setEnabled(ok && sh->runStatus());
		ui.actionCopy_link->setEnabled(ok);
	}
}

void neteK::Gui::globalSettings()
{
	GlobalSettings().exec();
}

void neteK::Gui::copyLinkMenu(int id)
{
	QPointer<Share> sh = getShare(id);
	if(validAndConfigured(sh))
		CopyLinkMenu(sh->URLScheme(), sh->port()).exec(QCursor::pos());
}

void neteK::Gui::showLog()
{
	if(m_log_viewer) {
		m_log_viewer->activateWindow();
		m_log_viewer->raise();
	} else {
		m_log_viewer = new LogViewer;
		m_log_viewer->setAttribute(Qt::WA_DeleteOnClose);
		m_log_viewer->show();
	}
}

void neteK::Gui::showAbout()
{
	About().exec();
}

void neteK::Gui::quitRequest()
{
	if(m_tray_icon)
		m_tray_icon->deleteLater();
		
	emit userQuit();
}

#include "netek_gui.moc"
