#include "netek_gui.h"
#include "netek_share.h"
#include "netek_shares.h"
#include "netek_globalsettings.h"
#include "netek_trayicon.h"
#include "netek_settings.h"
#include "netek_netutils.h"

// TODO 1.1: zeroconf
// TODO 1.1: upnp
// TODO 1.1: add logging of events
// TODO 1.0: mac os x port

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
		return QString("%1://%2:%3")
			.arg(m_protocol)
			.arg(addr.toString())
			.arg(m_port);
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
		}
	}
	
public slots:
	void resolved(QHostAddress addr)
	{
		if(!m_public)
			return;
			
		if(addr.isNull())
			m_public->setText(tr("Autodetect failed"));
		else
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
		
		setTitle(tr("Copy &link"));
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

	m_icon = makeTrayIcon(this);
	if(m_icon) {
		m_icon->connect(this, SIGNAL(quit()), SLOT(deleteLater()));
		connect(m_icon, SIGNAL(activated()), SLOT(toggleVisible()));
		connect(m_icon, SIGNAL(showMenu(const QPoint &)), SLOT(trayMenu(const QPoint &)));
	}

	m_shares = new Shares;
	m_shares->setParent(this);
	
	ui.shareList->setContextMenuPolicy(Qt::CustomContextMenu);
	
	connect(ui.shareList, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(shareMenu(const QPoint &)));

	connect(m_shares, SIGNAL(changed()), SLOT(sharesChanged()));

	connect(ui.shareList, SIGNAL(itemSelectionChanged()), SLOT(sharesChanged()), Qt::QueuedConnection);
	connect(ui.shareList, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(toggleRunStatus()));

	connect(ui.create, SIGNAL(clicked()), m_shares, SLOT(createShareWithSettings()));
	connect(ui.deleteShare, SIGNAL(clicked()), SLOT(deleteShare()));
	connect(ui.settings, SIGNAL(clicked()), SLOT(shareSettings()));
	connect(ui.start, SIGNAL(clicked()), SLOT(startShare()));
	connect(ui.stop, SIGNAL(clicked()), SLOT(stopShare()));
	connect(ui.copyLink, SIGNAL(clicked()), SLOT(copyLinkMenu()));

	connect(ui.action_FTP_shares, SIGNAL(triggered()), SLOT(toggleVisible()));
	connect(ui.action_Create_share, SIGNAL(triggered()), m_shares, SLOT(createShareWithSettings()));
	connect(ui.actionSettings, SIGNAL(triggered()), SLOT(shareSettings()));
	connect(ui.actionStart, SIGNAL(triggered()), SLOT(startShare()));
	connect(ui.actionStop, SIGNAL(triggered()), SLOT(stopShare()));
	connect(ui.actionDelete, SIGNAL(triggered()), SLOT(deleteShare()));
	connect(ui.action_Global_settings, SIGNAL(triggered()), SLOT(globalSettings()));
	connect(ui.action_Quit, SIGNAL(triggered()), SIGNAL(quit()));

	connect(this, SIGNAL(quit()), qApp, SLOT(userQuit()));

	sharesChanged();
	
	{
		QRect geom = Settings().guiGeometry();
		if(geom.isValid()) {
			resize(geom.size());
			move(geom.topLeft());
		}
	}

	if(!m_icon)
		show();
}

void neteK::Gui::closeEvent(QCloseEvent *e)
{
	if(m_icon) {
		hide();
		e->ignore();
	} else
		emit quit();
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
	m_save_geometry_timer = false;
	Settings().setGuiGeometry(QRect(pos(), size()));
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

void neteK::Gui::trayMenu(const QPoint &pos)
{
	QMenu menu(qApp->applicationName());
	menu.addAction(ui.action_FTP_shares);
	menu.setDefaultAction(ui.action_FTP_shares);
	menu.addAction(ui.action_Create_share);
	menu.addAction(ui.action_Global_settings);
	menu.addAction(ui.action_Quit);

	menu.exec(pos);
}
	
void neteK::Gui::shareMenu(const QPoint &)
{
	QPointer<Share> sh = currentShare();
	if(sh && sh->status() != Share::StatusUnconfigured) {
		CopyLinkMenu cmenu(sh->URLProtocol(), sh->port());
		QMenu menu;
		menu.addMenu(&cmenu);
			
		menu.addAction(ui.actionSettings);
		menu.addAction(ui.actionStart);
		menu.addAction(ui.actionStop);
		menu.addAction(ui.actionDelete);
		
		ui.actionStart->setEnabled(!sh->runStatus());
		ui.actionStop->setEnabled(sh->runStatus());
		
		menu.exec(QCursor::pos());
	}
}

neteK::Share *neteK::Gui::currentShare()
{
	return m_shares->share(ui.shareList->currentRow());
}

void neteK::Gui::shareSettings()
{
	QPointer<Share> sh = currentShare();
	if(sh)
		sh->showSettings();
}

void neteK::Gui::deleteShare()
{
	m_shares->deleteShareWithQuestion(ui.shareList->currentRow());
}

void neteK::Gui::startShare()
{
	QPointer<Share> sh = currentShare();
	if(sh)
		sh->startIfStopped();
}

void neteK::Gui::stopShare()
{
	QPointer<Share> sh = currentShare();
	if(sh)
		sh->stop();
}

void neteK::Gui::toggleRunStatus()
{
	QPointer<Share> sh = currentShare();
	if(sh) {
		if(sh->runStatus())
			sh->stop();
		else
			sh->startIfStopped();
	}
}

void neteK::Gui::sharesChanged()
{
	while(ui.shareList->count() > m_shares->shares())
		delete ui.shareList->takeItem(ui.shareList->count()-1);

	for(int i=0; i<m_shares->shares(); ++i) {
		QPointer<Share> sh = m_shares->share(i);
		if(sh) {
			QListWidgetItem *item;
			if(i < ui.shareList->count()) {
				item = ui.shareList->item(i);
				Q_ASSERT(item);
			} else {
				item = new QListWidgetItem;
				ui.shareList->addItem(item);
				QFont font = item->font();
				font.setBold(true);
				item->setFont(font);
			}

			QString desc(sh->niceId());
			
			{
				QString flags;
				if(sh->readOnly())
					flags += 'R';
				if(sh->access() == Share::AccessUsernamePassword)
					flags += 'U';
				if(flags.size())
					desc += QString(" (%1)").arg(flags);
				else
					desc += " (-)";
			}
			
			desc += tr(", ");

			switch(sh->status()) {
				case Share::StatusStarted:
					item->setIcon(QPixmap(":/icons/folder_open.png"));
					desc += tr("started");
					break;
				case Share::StatusStopped:
					item->setIcon(QPixmap(":/icons/folder_grey.png"));
					desc += tr("stopped");
					break;
				default:
					item->setIcon(QPixmap(":/icons/exec.png"));
					desc += tr("processing");
			}

			if(sh->clients())
				desc += QString(", clients: %1").arg(sh->clients());

			item->setText(desc);
		}
	}

	{
		QPointer<Share> sh = currentShare();
		bool ok = sh && sh->status() != Share::StatusUnconfigured;
		ui.deleteShare->setEnabled(ok);
		ui.settings->setEnabled(ok);
		ui.start->setEnabled(ok && !sh->runStatus());
		ui.stop->setEnabled(ok && sh->runStatus());
		ui.copyLink->setEnabled(ok);
	}
}

void neteK::Gui::globalSettings()
{
	GlobalSettings().exec();
}

void neteK::Gui::copyLinkMenu()
{
	QPointer<Share> sh = currentShare();
	if(sh) {
		CopyLinkMenu menu(sh->URLProtocol(), sh->port());
		menu.exec(QCursor::pos());
	}
}

#include "netek_gui.moc"
