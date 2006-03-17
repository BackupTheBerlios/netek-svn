#include "netek_settings.h"
#include "netek_share.h"
#include "netek_sharesettings.h"
#include "netek_netutils.h"

neteK::ShareSettings::ShareSettings(Share *share)
: m_share(share)
{
	ui.setupUi(this);

	connect(ui.folderBrowse, SIGNAL(clicked()), SLOT(folderBrowse()));
	connect(ui.portPickRandom, SIGNAL(clicked()), SLOT(pickRandom()));

	ui.folder->setText(m_share->folder());

	if(m_share->port() == 0) {
		Settings settings;
		ui.port->setValue(settings.randomTcpPortMin());
		pickRandom();
	} else
		ui.port->setValue(m_share->port());

	ui.readOnly->setChecked(m_share->readOnly());
	ui.anonymous->setChecked(m_share->access() == Share::AccessAnonymous);
	ui.authentication->setChecked(m_share->access() == Share::AccessUsernamePassword);
	connect(ui.anonymous, SIGNAL(clicked()), SLOT(securityRadio()));
	connect(ui.authentication, SIGNAL(clicked()), SLOT(securityRadio()));

	{
		QString u, p;
		m_share->usernamePassword(u,p);
		ui.username->setText(u);
		ui.password->setText(p);
	}

	securityRadio();
}

void neteK::ShareSettings::securityRadio()
{
	bool enabled = ui.authentication->isChecked();

	ui.username->setEnabled(enabled);
	ui.password->setEnabled(enabled);
	ui.usernameLabel->setEnabled(enabled);
	ui.passwordLabel->setEnabled(enabled);
}

void neteK::ShareSettings::folderBrowse()
{
	QString name = QFileDialog::getExistingDirectory(this, tr("Select share folder"), ui.folder->text());
	if(name.size())
		ui.folder->setText(name);
}

void neteK::ShareSettings::pickRandom()
{
	for(int i=0; i<100; ++i) {
		quint16 port = randomPort();

		if(m_test_server)
			delete m_test_server;
		m_test_server = new QTcpServer(this);

		if(m_test_server->listen(QHostAddress::Any, port)) {
			ui.port->setValue(port);
			return;
		}
	}

	QMessageBox::warning(this, qApp->applicationName(), tr("Unable to bind any TCP port. Check random TCP port settings."), QMessageBox::Cancel, 0, 0);
}

void neteK::ShareSettings::accept()
{
	if(ui.folder->text().size() == 0) {
		QMessageBox::information(this, qApp->applicationName(), tr("Folder name must be entered."), QMessageBox::Cancel);
		return;
	}

	if(ui.authentication->isChecked() &&
		(ui.username->text().size() == 0 || ui.password->text().size() == 0))
	{
		QMessageBox::information(this, qApp->applicationName(), tr("Username and password must be entered."), QMessageBox::Cancel);
		return;
	}

	m_share->setFolder(ui.folder->text());
	m_share->setPort(ui.port->value());

	m_share->setReadOnly(ui.readOnly->isChecked());

	if(ui.authentication->isChecked()) {
		m_share->setAccess(Share::AccessUsernamePassword);
		m_share->setUsernamePassword(ui.username->text(), ui.password->text());
	} else {
		m_share->setUsernamePassword("", "");
		m_share->setAccess(Share::AccessAnonymous);
	}

	QDialog::accept();
}
