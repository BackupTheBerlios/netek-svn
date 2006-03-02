#include "netek_globalsettings.h"
#include "netek_settings.h"

neteK::GlobalSettings::GlobalSettings()
{
	ui.setupUi(this);
	
	Settings settings;
	ui.tcpFrom->setValue(settings.randomTcpPortMin());
	ui.tcpTo->setValue(settings.randomTcpPortMax());
	ui.ftpUseUnicode->setChecked(settings.ftpUseUnicodeByDefault());
	ui.ftpAllowPassive->setChecked(settings.ftpAllowPassive());
}

void neteK::GlobalSettings::accept()
{
	if(ui.tcpFrom->value() > ui.tcpTo->value()) {
		QMessageBox::information(this, qApp->applicationName(), tr("Invalid TCP port range."), QMessageBox::Cancel);
		return;
	}
	
	Settings settings;
	settings.setRandomTcpPortMin(ui.tcpFrom->value());
	settings.setRandomTcpPortMax(ui.tcpTo->value());
	settings.setFtpUseUnicodeByDefault(ui.ftpUseUnicode->isChecked());
	settings.setFtpAllowPassive(ui.ftpAllowPassive->isChecked());
	
	QDialog::accept();
}
