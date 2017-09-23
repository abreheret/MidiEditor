/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AboutDialog.h"
#include "version.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QVariant>
#include <QTextBrowser>
#include <QScrollArea>
#include <QtGlobal>

AboutDialog::AboutDialog(QWidget *parent):QDialog(parent) {

	setMinimumWidth(550);
	setMaximumHeight(450);
	setWindowTitle(tr("About"));
	setWindowIcon(QIcon("graphics/icon.png"));
	QGridLayout *layout = new QGridLayout(this);

	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(":/run_environment/graphics/midieditor.png").scaledToWidth(80, Qt::SmoothTransformation));
	icon->setFixedSize(80, 80);
	layout->addWidget(icon, 0, 0, 3, 1);

	QLabel *title = new QLabel("<h1>"+QApplication::applicationName()+"</h1>", this);
	layout->addWidget(title, 0, 1, 1, 2);
	title->setStyleSheet("color: black");

	QLabel *version = new QLabel("Version: " + QString(GIT_TAG) + " (" + QApplication::instance()->property("arch").toString() + "-Bit, " + QString(GIT_DATE) + ")", this);
	layout->addWidget(version, 1, 1, 1, 2);
	version->setStyleSheet("color: black");

	QScrollArea *a = new QScrollArea(this);
	QLabel *content = new QLabel(
	"<html>"
		"<body>"
		"<p>"
			"<a href=\"http://midieditor.sourceforge.net/\">www.midieditor.sourceforge.net</a><br>"
			"mailto:bugs@markus-schwenk.de<br>"
			"<a href=\"https://github.com/abreheret/MidiEditor/\">https://github.com/abreheret/MidiEditor</a><br>"
			"abreheret@gmail.com"
		"</p>"
		"<h3>Authors</h3>"
		"<p>"
			"<strong>Markus Schwenk</strong>"
			"<ul style=\"list-style: none\">"
				"<li>Email: kontakt@markus-schwenk.de</li>"
				"<li>Website: <a href=\"http://www.markus-schwenk.de\">www.markus-schwenk.de</a></li>"
			"</ul>"
			"<strong>Amaury Br&eacute;h&eacute;ret</strong>"
			"<ul style=\"list-style: none\">"
				"<li>Email: abreheret@gmail.com</li>"
				"<li>Github: <a href=\"https://github.com/abreheret\">https://github.com/abreheret</a></li>"
			"</ul>"
		"</p>"
		"<h3>Thanks to</h3>"
		"<p>Romain Behar</p>"
		"<h3>Credits</h3>"
		"<p>"
			"3D icons by Double-J Design (http://www.doublejdesign.co.uk)<br>"
			"Flat icons designed by Freepik<br>"
			"Metronome sound by Mike Koenig"
		"</p>"
		"<h3>Third party Libraries</h3>"
		"<p>"
			"Qt (Copyright (c) 2017 The Qt Company Ltd.)<br>"
			"RtMidi (Copyright (c) 2003-2017 Gary P. Scavone)"
		"</p>"
		"<p>"
		"<h3>License</h3>"
				"<p style=\"background: #eeeeee; white-space: pre; box-sizing: padding-box; padding: 1em; margin: 1em;\">"
				"<pre style=\"\">"
				"Copyright (C) 2017 Markus Schwenk\n"
				"Copyright (C) 2017 Amaury Br&eacute;h&eacute;ret\n"
				"\n"
				"This program is free software: you can redistribute it and/or modify\n"
				"it under the terms of the GNU General Public License as published by\n"
				"the Free Software Foundation, either version 3 of the License, or\n"
				"(at your option) any later version.\n"
				"\n"
				"This program is distributed in the hope that it will be useful,\n"
				"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				"GNU General Public License for more details.\n"
				"\n"
				"You should have received a copy of the GNU General Public License\n"
				"along with this program.  If not, see &lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;."
				"</pre>"
				"</p>"
			"</p>"
		"</body>"
	"</html>"
								 );
	a->setWidgetResizable(true);
	a->setWidget(content);
	a->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	a->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	layout->addWidget(a, 2, 1, 2, 2);
	content->setStyleSheet("color: black; background-color: white; padding: 5px");

	content->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	content->setOpenExternalLinks(true);

	layout->setRowStretch(3, 1);
	layout->setColumnStretch(1, 1);

	QFrame *f = new QFrame( this );
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f, 4, 0, 1, 3);

	QPushButton *close = new QPushButton("Close");
	layout->addWidget(close, 5, 2, 1, 1);
	connect(close, SIGNAL(clicked()), this, SLOT(hide()));
}
