#include "netek_globalsettings.h"
#include "netek_settings.h"

neteK::GlobalSettings::GlobalSettings()
{
	ui.setupUi(this);
	
	Settings settings;
	ui.publicAddress->setText(settings.publicAddress());
	ui.tcpFrom->setValue(settings.randomTcpPortMin());
	ui.tcpTo->setValue(settings.randomTcpPortMax());
}

void neteK::GlobalSettings::accept()
{
	if(ui.tcpFrom->value() > ui.tcpTo->value()) {
		QMessageBox::information(this, qApp->applicationName(), tr("Invalid TCP port range."), QMessageBox::Cancel);
		return;
	}
	
	Settings settings;
	settings.setPublicAddress(ui.publicAddress->text());
	settings.setRandomTcpPortMin(ui.tcpFrom->value());
	settings.setRandomTcpPortMax(ui.tcpTo->value());
	
	QDialog::accept();
}
