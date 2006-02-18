#include "netek_gui.h"
#include "netek_share.h"
#include "netek_shares.h"
#include "netek_globalsettings.h"
#include "netek_trayicon.h"

// TODO: store last window position in settings

neteK::Gui::Gui()
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

	ui.shareList->addAction(ui.actionSettings);
	ui.shareList->addAction(ui.actionStart);
	ui.shareList->addAction(ui.actionStop);
	ui.shareList->addAction(ui.actionDelete);
	ui.shareList->setContextMenuPolicy(Qt::ActionsContextMenu);

	connect(m_shares, SIGNAL(changed()), SLOT(sharesChanged()));

	connect(ui.shareList, SIGNAL(itemSelectionChanged()), SLOT(sharesChanged()), Qt::QueuedConnection);
	connect(ui.shareList, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(toggleRunStatus()));

	connect(ui.create, SIGNAL(clicked()), m_shares, SLOT(createShareWithSettings()));
	connect(ui.deleteShare, SIGNAL(clicked()), SLOT(deleteShare()));
	connect(ui.settings, SIGNAL(clicked()), SLOT(shareSettings()));
	connect(ui.start, SIGNAL(clicked()), SLOT(startShare()));
	connect(ui.stop, SIGNAL(clicked()), SLOT(stopShare()));

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
		ui.actionDelete->setEnabled(sh);
		ui.actionSettings->setEnabled(sh);
		ui.actionStart->setEnabled(sh && !sh->runStatus());
		ui.actionStop->setEnabled(sh && sh->runStatus());
		ui.deleteShare->setEnabled(sh);
		ui.settings->setEnabled(sh);
		ui.start->setEnabled(sh && !sh->runStatus());
		ui.stop->setEnabled(sh && sh->runStatus());
	}
}

void neteK::Gui::globalSettings()
{
	GlobalSettings().exec();
}
