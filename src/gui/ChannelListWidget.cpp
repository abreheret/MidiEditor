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

#include "ChannelListWidget.h"
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QPainter>

#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../protocol/Protocol.h"

#define ROW_HEIGHT 85

ChannelListItem::ChannelListItem(int ch, ChannelListWidget *parent) : QWidget(parent){

	channelList = parent;
	channel = ch;

	setContentsMargins(0, 0, 0, 0);
	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);
	layout->setVerticalSpacing(1);

	colored = new ColoredWidget(*(MidiChannel::colorByChannelNumber(channel)), this);
	layout->addWidget(colored, 0, 0, 2, 1);
	QString text = "Channel "+QString::number(channel);
	if(channel==16){
		text = "General Events";
	}
	layout->addWidget(new QLabel(text, this), 0, 1, 1, 1);

	instrumentLabel = new QLabel("none", this);
	layout->addWidget(instrumentLabel, 1, 1, 1, 1);

	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(12, 12));

	// visibility
	visibleAction = new QAction(QIcon("graphics/channelwidget/visible.png"), "Channel visible", toolBar);
	visibleAction->setCheckable(true);
	visibleAction->setChecked(true);
	toolBar->addAction(visibleAction);
	connect(visibleAction, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));

	// audibility
	if(channel<16){
		loudAction = new QAction(QIcon("graphics/channelwidget/loud.png"), "Channel audible", toolBar);
		loudAction->setCheckable(true);
		loudAction->setChecked(true);
		toolBar->addAction(loudAction);
		connect(loudAction, SIGNAL(toggled(bool)), this, SLOT(toggleAudibility(bool)));

		// solo
		soloAction = new QAction(QIcon("graphics/channelwidget/solo.png"), "Solo mode", toolBar);
		soloAction->setCheckable(true);
		soloAction->setChecked(false);
		toolBar->addAction(soloAction);
		connect(soloAction, SIGNAL(toggled(bool)), this, SLOT(toggleSolo(bool)));

		toolBar->addSeparator();

		// instrument
		QAction *instrumentAction = new QAction(QIcon("graphics/channelwidget/instrument.png"), "Select instrument", toolBar);
		toolBar->addAction(instrumentAction);
		connect(instrumentAction, SIGNAL(triggered()), this, SLOT(instrument()));
	}

	layout->addWidget(toolBar, 2, 1, 1, 1);

	layout->setRowStretch(2,1);
	setContentsMargins(5, 1, 5, 0);
	setFixedHeight(ROW_HEIGHT);
}

void ChannelListItem::toggleVisibility(bool visible){
	channelList->midiFile()->protocol()->startNewAction("change visibility");
	channelList->midiFile()->channel(channel)->setVisible(visible);
	channelList->midiFile()->protocol()->endAction();
	emit channelStateChanged();
}

void ChannelListItem::toggleAudibility(bool audible){
	QString text = "Muted channel";
	if(audible){
		text = "Unmuted channel";
	}
	channelList->midiFile()->protocol()->startNewAction(text);
	channelList->midiFile()->channel(channel)->setMute(!audible);
	channelList->midiFile()->protocol()->endAction();
	emit channelStateChanged();
}

void ChannelListItem::toggleSolo(bool solo){
	QString text = "Entered solomode";
	if(!solo){
		text = "Exited solomode";
	}
	channelList->midiFile()->protocol()->startNewAction(text);
	channelList->midiFile()->channel(channel)->setSolo(solo);
	channelList->midiFile()->protocol()->endAction();
	emit channelStateChanged();
}

void ChannelListItem::instrument(){
	emit selectInstrumentClicked(channel);
}

void ChannelListItem::onBeforeUpdate() {

	QString text = MidiFile::instrumentName(channelList->midiFile()->channel(channel)->
			progAtTick(channelList->midiFile()->cursorTick()));
	if(channel==16){
		text = "Events affecting all channels";
	}
	instrumentLabel->setText(text);

	if(channelList->midiFile()->channel(channel)->eventMap()->isEmpty()){
		colored->setColor(Qt::lightGray);
	} else {
		colored->setColor(*(MidiChannel::colorByChannelNumber(channel)));
	}

	if(visibleAction->isChecked() != channelList->midiFile()->channel(channel)->visible()){
		disconnect(visibleAction, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));
		visibleAction->setChecked(channelList->midiFile()->channel(channel)->visible());
		connect(visibleAction, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));
		emit channelStateChanged();
	}

	if(channel < 16 && (loudAction->isChecked()) == (channelList->midiFile()->channel(channel)->mute())){
		disconnect(loudAction, SIGNAL(toggled(bool)), this, SLOT(toggleAudibility(bool)));
		loudAction->setChecked(!channelList->midiFile()->channel(channel)->mute());
		connect(loudAction, SIGNAL(toggled(bool)), this, SLOT(toggleAudibility(bool)));
		emit channelStateChanged();
	}

	if(channel < 16 && (soloAction->isChecked()) != (channelList->midiFile()->channel(channel)->solo())){
		disconnect(soloAction, SIGNAL(toggled(bool)), this, SLOT(toggleSolo(bool)));
		soloAction->setChecked(channelList->midiFile()->channel(channel)->solo());
		connect(soloAction, SIGNAL(toggled(bool)), this, SLOT(toggleSolo(bool)));
		emit channelStateChanged();
	}
}

ChannelListWidget::ChannelListWidget(QWidget *parent) : QListWidget(parent) {

	setSelectionMode(QAbstractItemView::NoSelection);

	for(int channel = 0; channel <17; channel++){
		ChannelListItem *widget = new ChannelListItem(channel, this);
		QListWidgetItem *item = new QListWidgetItem();
		item->setSizeHint(QSize(0,ROW_HEIGHT));
		addItem(item);
		setItemWidget(item,widget);
		items.append(widget);

		setStyleSheet( "QListWidget::item { border-bottom: 1px solid lightGray; }" );

		connect(widget, SIGNAL(channelStateChanged()), this, SIGNAL(channelStateChanged()));
		connect(widget, SIGNAL(selectInstrumentClicked(int)), this, SIGNAL(selectInstrumentClicked(int)));
	}

	file = 0;
}

void ChannelListWidget::setFile(MidiFile *f){
	file = f;
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(update()));
	update();
}

void ChannelListWidget::update(){

	foreach(ChannelListItem *item, items){
		item->onBeforeUpdate();
	}

	QListWidget::update();
}

/*
void ChannelListWidget::paintEvent(QPaintEvent *event){

	if(!file){
		return;
	}

	int instrumentChangeChannel = -1;

	QPainter *painter = new QPainter(this);
	QFont f = painter->font();
	f.setPixelSize(12);
	painter->setFont(f);
	painter->fillRect(0, 0, width(), height(), Qt::white);

	for(int i = 0; i<17; i++){

		bool mouseIn = mouseInRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT);
		if(mouseIn && enabled){
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(234,246,255));
		} else {
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(194,230,255));
		}

		if(file->channel(i)->eventMap()->isEmpty()){
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(200,200,200));
		}
		painter->drawLine(0,LINE_HEIGHT*i, width(), LINE_HEIGHT*i);

		int y = LINE_HEIGHT*i;
		painter->fillRect(5, y+5, 16, 16, *(file->channel(i)->color()));

		painter->drawLine(5,y+5, 21, y+5);
		painter->drawLine(5,y+5, 5, y+21);
		painter->drawLine(21, y+21, 5, y+21);
		painter->drawLine(21, y+21, 21, y+5);

		QString text = "Channel "+QString::number(i);
		if(i==16){
			text = "General Events";
		}
		painter->drawText(25, y+17, text);
		text = MidiFile::instrumentName(file->channel(i)->
				progAtTick(file->cursorTick()));
		if(i==16){
			text = "Events affecting all channels";
		}
		painter->drawText(25, y+30, text);

		text = "";

		if(file->channel(i)->visible()){
			painter->drawImage(6, y+35,
					QImage("graphics/channelwidget/visible.png"));
		} else {
			painter->drawImage(6, y+35,
					QImage("graphics/channelwidget/hidden.png"));
			painter->fillRect(6, y+35, 12, 12, QColor(0,0,0, 60));
		}
		if(enabled && mouseInRect(6, y+35, 12, 12)){
			painter->fillRect(6, y+35, 12, 12, QColor(0,0,0, 100));
			if(file->channel(i)->visible()){
				text = "hide channel";
			} else {
				text = "set visible";
			}
			if(mouseReleased && enabled){
				file->protocol()->startNewAction("change visibility");
				file->channel(i)->setVisible(!file->channel(i)->visible());
				file->protocol()->endAction();
			}
		}

		if(i<16){
			if(!file->channel(i)->mute()){
				painter->drawImage(22, y+35,
						QImage("graphics/channelwidget/loud.png"));
			} else {
				painter->drawImage(22, y+35,
						QImage("graphics/channelwidget/mute.png"));
				painter->fillRect(22, y+35, 12, 12, QColor(0,0,0, 60));
			}
			if(mouseInRect(22, y+35, 12, 12)){
				painter->fillRect(22, y+35, 12, 12, QColor(0,0,0, 100));
				if(file->channel(i)->mute()){
					text = "make audible";
				} else {
					text = "mute channel";
				}
				if(mouseReleased && enabled){
					file->protocol()->startNewAction(text);
					file->channel(i)->setMute(!file->channel(i)->mute());
					file->protocol()->endAction();
				}
			}
			painter->drawImage(54, y+35, QImage("graphics/channelwidget/solo.png"));
			if(!file->channel(i)->solo()){
				painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 60));
			}
			if(enabled && mouseInRect(54, y+35, 12, 12)){
				painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 100));
				if(!file->channel(i)->solo()){
					text = "enter solomode";
				} else {
					text = "exit solomode";
				}
				if(mouseReleased && enabled){
					file->protocol()->startNewAction(text);
					for(int c = 0; c<17; c++){
						file->channel(c)->setSolo(c==i&&!file->channel(c)->solo());
					}
					file->protocol()->endAction();
				}
			}


			painter->drawImage(38, y+35,
				QImage("graphics/channelwidget/instrument.png"));
			if(enabled && mouseInRect(38, y+35, 12, 12)){
				painter->fillRect(38, y+35, 12, 12, QColor(0,0,0, 100));
				text = "select instrument";
				if(mouseReleased){
					instrumentChangeChannel = i;
				}
			}

			painter->drawText(70, y+46, text);
		} else {
			painter->drawText(22, y+46, text);
		}

	}

	if(instrumentChangeChannel > -1){
		emit selectInstrumentClicked(instrumentChangeChannel);
		instrumentChangeChannel = -1;
	}
	painter->drawLine(0,0,width()-1,0);
	painter->drawLine(width()-1, 0, width()-1, height()-1);
	painter->drawLine(width()-1, height()-1, 0, height()-1);
	painter->drawLine(0, height()-1, 0, 0);

	if(!enabled){
		painter->fillRect(0,0,width(), height(), QColor(110, 110, 110, 100));
	}
	if(mouseReleased && enabled){
		emit(channelStateChanged());
		mouseReleased = false;
		delete painter;
		update();
		return;
	}
	delete painter;
} */

MidiFile *ChannelListWidget::midiFile(){
	return file;
}

ColoredWidget::ColoredWidget(QColor color, QWidget *parent) : QWidget(parent) {
	_color = color;
	setFixedSize(30, 30);
	setContentsMargins(0,0,0,0);
}

void ColoredWidget::paintEvent(QPaintEvent *event){
	QPainter p;
	p.begin(this);
	p.fillRect(0,0,width(), height(), Qt::white);
	p.setPen(Qt::lightGray);
	p.setBrush(_color);
	p.drawRoundRect(0,0,width()-1, height()-1, 9, 9);
	p.end();
}
