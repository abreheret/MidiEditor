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

#include "DonateDialog.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

DonateDialog::DonateDialog(QWidget *parent):QDialog(parent)
{
	setWindowIcon(QIcon("graphics/icon.png"));

	QGridLayout *layout = new QGridLayout;

	//Label starttext
	QLabel *title = new QLabel("<h2><u>Donate</u></h2><font" \
		" color='#666666'>Please donate if this software is useful for you. " \
		 "<br>To donate visit my Donate Page on <a href=\"http://sourceforge.net/project/project_donations.php?group_id=370293\">SourceForge.net</a></font>");
	title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	title->setOpenExternalLinks(true);
	layout->addWidget(title, 0,0,1,4);

	// button close
	QPushButton *close = new QPushButton("Close");
	layout->addWidget(close, 1, 3, 1, 1);
	connect(close, SIGNAL(clicked()), this, SLOT(hide()));

	setLayout(layout);
	setWindowTitle(tr("Donate"));
}
