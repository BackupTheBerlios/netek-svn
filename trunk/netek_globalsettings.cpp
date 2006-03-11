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
	ui.paAutodetect->setChecked(settings.publicAddress() == Settings::PublicAddressAutodetect);
	ui.paManual->setChecked(settings.publicAddress() == Settings::PublicAddressManual);
	ui.publicAddress->setText(settings.customPublicAddress());
	
	connect(ui.paAutodetect, SIGNAL(clicked()), SLOT(publicAddressRadio()));
	connect(ui.paManual, SIGNAL(clicked()), SLOT(publicAddressRadio()));
	
	publicAddressRadio();
}

void neteK::GlobalSettings::publicAddressRadio()
{
	ui.publicAddress->setEnabled(ui.paManual->isChecked());
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
	settings.setPublicAddress(
		ui.paManual->isChecked()
			? Settings::PublicAddressManual
			: Settings::PublicAddressAutodetect);
	settings.setCustomPublicAddress(ui.publicAddress->text());
	
	QDialog::accept();
}
