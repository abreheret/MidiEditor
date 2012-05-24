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

#include "TextEvent.h"
#include <QLayout>

TextEvent::TextEvent(int channel) : MidiEvent(channel) {
	_type = TEXT;
	_text = "";
}

QString TextEvent::text(){
	return _text;
}

void TextEvent::setText(QString text){
	_text = text;
}

int TextEvent::type(){
	return _type;
}

void TextEvent::setType(int type){
	_type = type;
}

int TextEvent::line(){
	return TEXT_EVENT_LINE;
}

QByteArray TextEvent::save(){
	// TODO
}

QString TextEvent::typeString(){
	return "Text Event";
}

// Widgets for EventWidget
QTextEdit *TextEvent::_text_area = 0;
QLabel *TextEvent::_text_label = 0;
QComboBox *TextEvent::_type_combo = 0;
QLabel *TextEvent::_type_label = 0;

void TextEvent::generateWidget(QWidget *widget){

	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_text_area == 0) _text_area = new QTextEdit();
	if(_text_label == 0) _text_label = new QLabel();
	if(_type_label == 0) _type_label = new QLabel();

	if(_type_combo == 0){
		_type_combo = new QComboBox();
		_type_combo->addItem("General text");
		_type_combo->addItem("Copyright");
		_type_combo->addItem("Trackname");
		_type_combo->addItem("Instrument name");
		_type_combo->addItem("Lyric");
		_type_combo->addItem("Marker");
		_type_combo->addItem("Comment");
	}

	// set Parents
	_type_combo->setParent(widget);
	_type_label->setParent(widget);
	_text_label->setParent(widget);
	_text_area->setParent(widget);

	// unhide
	_type_combo->setHidden(false);
	_type_label->setHidden(false);
	_text_label->setHidden(false);
	_text_area->setHidden(false);

	// type
	_type_label->setText("Type:");
	_type_combo->setCurrentIndex(_type-1);

	// text
	_text_label->setText("Text:");
	_text_area->setText(_text);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_type_label);
	layout->addWidget(_type_combo);
	layout->addWidget(_text_label);
	layout->addWidget(_text_area);
}
