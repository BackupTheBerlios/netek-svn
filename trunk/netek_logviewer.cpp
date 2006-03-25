#include "netek_logviewer.h"
#include "netek_application.h"
#include "netek_settings.h"

// TODO: speedup text widget

neteK::LogViewer::LogViewer()
{
	ui.setupUi(this);
	
	Settings settings;
	
	ui.logKBytes->setValue(settings.logKBytes());
	
	connect(this, SIGNAL(scrollDownSignal()), SLOT(scrollDown()), Qt::QueuedConnection);
	connect(Application::log(), SIGNAL(appendToLog(QString)), SLOT(log(QString)));
	connect(ui.copyToClipboard, SIGNAL(clicked()), SLOT(copyToClipboard()));
	connect(ui.saveToFile, SIGNAL(clicked()), SLOT(saveToFile()));
	connect(ui.close, SIGNAL(clicked()), SLOT(reject()));
	
	{
		QRect geom = settings.logViewerGeometry();
		if(geom.isValid()) {
			resize(geom.size());
			move(geom.topLeft());
		}
	}
	
	log(Application::log()->readLog());
}

void neteK::LogViewer::reject()
{
	Settings settings;
	settings.setLogKBytes(ui.logKBytes->value());
	settings.setLogViewerGeometry(QRect(pos(), size()));
	
	QDialog::reject();
}

void neteK::LogViewer::log(QString data)
{
	QTextCursor(ui.logText->document()->rootFrame()->lastCursorPosition())
		.insertText(data);
	
	if(ui.autoScroll->isChecked())
		emit scrollDownSignal();
}

void neteK::LogViewer::scrollDown()
{
	QPointer<QScrollBar> sb = ui.logText->verticalScrollBar();
	sb->setValue(sb->maximum());
}

void neteK::LogViewer::copyToClipboard()
{
	qApp->clipboard()->setText(ui.logText->toPlainText());
}

void neteK::LogViewer::saveToFile()
{
	QFileDialog dlg(this, tr("Save log"), QDir::current().absolutePath());
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.selectFile("netek_log.txt");
	if(QDialog::Accepted == dlg.exec()) {
		QByteArray txt = ui.logText->toPlainText().toUtf8();
		QFile f(dlg.selectedFiles().at(0));
		if(!f.open(QIODevice::WriteOnly) || txt.size() != f.write(txt))
			QMessageBox::critical(this, qApp->applicationName(), tr("Error saving file!"),
				QMessageBox::Cancel, 0);
	}
}
