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

#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"

TextEvent::TextEvent(int channel, MidiTrack *track) : MidiEvent(channel, track) {
	_type = TextTextEventType;
	_text = "";
}

TextEvent::TextEvent(const TextEvent &other) : MidiEvent(other) {
	_type = other._type;
	_text = other._text;
}

MidiEvent::EventType TextEvent::type() const {
	return TextEventType;
}

QString TextEvent::text(){
	return _text;
}

void TextEvent::setText(QString text){
	_text = text;
	protocol(copy(), this);
}

TextEvent::TextType TextEvent::textType(){
	return _type;
}

void TextEvent::setTextType(TextEvent::TextType type){
	_type = type;
	protocol(copy(), this);
}

int TextEvent::line(){
	return TEXT_EVENT_LINE;
}

QByteArray TextEvent::save(){
	mbstate_t mbs;
	mbrlen(NULL, 0, &mbs);

	QByteArray array = QByteArray();

	array.append(qint8(0xFF));
	array.append(qint8(_type));
	array.append(MidiFile::writeVariableLengthValue(_text.length()));

	wchar_t *text_wchar  = new wchar_t[_text.length()];
	_text.toWCharArray(text_wchar);

	for(int i = 0; i < _text.length(); i++){
		char buffer [16];
		wcrtomb(buffer, text_wchar[i], &mbs);
		array.append(buffer);
	}
	delete[] text_wchar;
	return array;
}

QString TextEvent::typeString(){
	return "Text Event";
}

ProtocolEntry *TextEvent::copy(){
	return new TextEvent(*this);
}

void TextEvent::reloadState(ProtocolEntry *entry){
	TextEvent *other = qobject_cast<TextEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_text = other->_text;
	_type = other->_type;
}

QString TextEvent::textTypeString(int type){
	switch(type){
		case TextTextEventType: return "General text";
		case CopyrightTextEventType: return "Copyright";
		case TrackNameTextEventType: return "Trackname";
		case InstrumentTextEventType: return "Instrument name";
		case LyricTextEventType: return "Lyric";
		case MarkerTextEventType: return "Marker";
		case CommentTextEventType: return "Comment";
	}
	return QString();
}
