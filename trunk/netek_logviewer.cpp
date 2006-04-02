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

#include "netek_logviewer.h"
#include "netek_application.h"
#include "netek_settings.h"

// TODO: speedup text widget

neteK::LogViewer::LogViewer()
{
	ui.setupUi(this);

	{
		QFont font = ui.logText->font();
#ifdef Q_OS_UNIX
		font.setFamily("Monospace");
#else
		font.setFamily("");
		font.setStyleHint(QFont::TypeWriter);
#endif
		ui.logText->setFont(font);
	}

	Settings settings;

	ui.logKBytes->setValue(settings.logKBytes());

	connect(this, SIGNAL(scrollDownSignal()), SLOT(scrollDown()), Qt::QueuedConnection);
	connect(Application::log(), SIGNAL(appendToLog(QString)), SLOT(log(QString)));
	connect(ui.copyToClipboard, SIGNAL(clicked()), SLOT(copyToClipboard()));
	connect(ui.saveToFile, SIGNAL(clicked()), SLOT(saveToFile()));
	connect(ui.close, SIGNAL(clicked()), SLOT(reject()));
	connect(ui.clearLog, SIGNAL(clicked()), SLOT(clearLog()));

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
		if(!f.open(QIODevice::WriteOnly | QIODevice::Text) || txt.size() != f.write(txt))
			QMessageBox::critical(this, qApp->applicationName(), tr("Error saving file!"),
				QMessageBox::Cancel, 0);
	}
}

void neteK::LogViewer::clearLog()
{
	Application::log()->clearLog();
	ui.logText->setPlainText(QString());
}
