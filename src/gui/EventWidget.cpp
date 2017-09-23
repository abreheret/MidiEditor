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

#include "EventWidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QtCore/qmath.h>
#include <QMessageBox>

#include "../MidiEvent/MidiEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../midi/MidiChannel.h"

#include "../MidiEvent/ChannelPressureEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/KeySignatureEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/PitchBendEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/SysExEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/UnknownEvent.h"

#include "DataEditor.h"

QSize EventWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
	EventWidget::EditorField field = static_cast<EventWidget::EditorField>(index.data(Qt::UserRole).toInt());
	if(field == EventWidget::MidiEventData){
		int min = 80;
		QSize normal = QStyledItemDelegate::sizeHint(option, index);
		if(normal.height()<min){
			return QSize(0, min);
		}
		return normal;
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}

}

QWidget *EventWidgetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{

	EventWidget::EditorField field = static_cast<EventWidget::EditorField>(index.data(Qt::UserRole).toInt());

	switch(field){
		case EventWidget::MidiEventTick: {
			return new QSpinBox(parent);
		}
		case EventWidget::MidiEventTrack: {
			return new QComboBox(parent);
		}
		case EventWidget::MidiEventNote: {
			return new QSpinBox(parent);
		}
		case EventWidget::NoteEventOffTick: {
			return new QSpinBox(parent);
		}
		case EventWidget::NoteEventVelocity: {
			return new QSpinBox(parent);
		}
		case EventWidget::NoteEventDuration: {
			return new QSpinBox(parent);
		}
		case EventWidget::MidiEventChannel: {
			return new QSpinBox(parent);
		}
		case EventWidget::MidiEventValue: {
			return new QSpinBox(parent);
		}
		case EventWidget::ControlChangeControl: {
			return new QComboBox(parent);
		}
		case EventWidget::ProgramChangeProgram: {
			return new QComboBox(parent);
		}
		case EventWidget::KeySignatureKey: {
			return new QComboBox(parent);
		}
		case EventWidget::TimeSignatureNum: {
			return new QSpinBox(parent);
		}
		case EventWidget::TimeSignatureDenom: {
			return new QComboBox(parent);
		}
		case EventWidget::TextType: {
			return new QComboBox(parent);
		}
		case EventWidget::TextText: {
			return new QTextEdit(parent);
		}
		case EventWidget::UnknownType: {
			return new QLineEdit(parent);
		}
		case EventWidget::MidiEventData: {
			return new DataEditor(parent);
		}
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void EventWidgetDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
	EventWidget::EditorField field = static_cast<EventWidget::EditorField>(index.data(Qt::UserRole).toInt());

	switch(field){
		case EventWidget::MidiEventTick: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(INT_MAX);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::MidiEventTrack: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int i = 0;
			foreach(MidiTrack *track, *(eventWidget->file()->tracks())){
				QString text = "Track "+QString::number(track->number())+": "+track->name();
				box->addItem(text);
				if(!text.compare(index.data().toString())){
					i = track->number();
				}
			}
			box->setCurrentIndex(i);
			break;
		}
		case EventWidget::MidiEventNote: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(127);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::NoteEventOffTick: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(INT_MAX);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::NoteEventVelocity: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(127);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::NoteEventDuration: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(INT_MAX);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::MidiEventChannel: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(0);
			spin->setMaximum(15);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::MidiEventValue: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMinimum(0);
			MidiEvent::EventType type = eventWidget->type();
			switch(type){
				case MidiEvent::PitchBendEventType: {
					spin->setMaximum(16383);
					break;
				}
				case MidiEvent::TempoChangeEventType: {
					spin->setMaximum(1000);
					break;
				}
				default: {
					spin->setMaximum(127);
					break;
				}
			}

			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::ControlChangeControl: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int current = 0;
			for(int i = 0; i<128; i++){
				QString text = QString::number(i)+": "+MidiFile::controlChangeName(i);
				box->addItem(text);
				if(!text.compare(index.data().toString())){
					current = i;
				}
			}
			box->setCurrentIndex(current);
			break;
		}
		case EventWidget::ProgramChangeProgram: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int current = 0;
			for(int i = 0; i<128; i++){
				QString text = QString::number(i)+": "+MidiFile::instrumentName(i);
				box->addItem(text);
				if(!text.compare(index.data().toString())){
					current = i;
				}
			}
			box->setCurrentIndex(current);
			break;
		}
		case EventWidget::KeySignatureKey: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int current = 0;
			int i = 0;
			foreach(QString key, eventWidget->keyStrings()){
				box->addItem(key);
				if(!key.compare(index.data().toString())){
					current = i;
				}
				i++;
			}
			box->setCurrentIndex(current);
			break;
		}
		case EventWidget::TimeSignatureNum: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			spin->setMaximum(1);
			spin->setMaximum(99);
			if(index.data().canConvert(QVariant::Int)){
				spin->setValue(index.data().toInt());
			}
			break;
		}
		case EventWidget::TimeSignatureDenom: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int current = 0;
			for(int p = 0; p<5; p++){
				box->addItem(QString::number((int)qPow(2, p)));
				if((index.data().toInt()) == ((int)qPow(2, p))){
					current = p;
				}
			}
			box->setCurrentIndex(current);
			break;
		}
		case EventWidget::TextType: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int current = 0;
			for(int p = 1; p<8; p++){
				box->addItem(TextEvent::textTypeString(p));
				if(!index.data().toString().compare(TextEvent::textTypeString(p))){
					current = p-1;
				}
			}
			box->setCurrentIndex(current);
			break;
		}
		case EventWidget::TextText: {
			QTextEdit *edit = qobject_cast<QTextEdit*>(editor);
			if(index.data().canConvert(QVariant::String)){
				edit->setPlainText(index.data().toString());
			}
			break;
		}
		case EventWidget::UnknownType: {
			QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
			edit->setInputMask("HH");
			if(index.data().canConvert(QVariant::String)){
				edit->setText(index.data().toString().right(2));
			}
			break;
		}
		case EventWidget::MidiEventData: {
			DataEditor *edit = qobject_cast<DataEditor*>(editor);
			if(index.data(Qt::UserRole+2).canConvert(QVariant::ByteArray)){
				edit->setData(index.data(Qt::UserRole+2).toByteArray());
			}
			break;
		}
	}
}

void EventWidgetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{

	EventWidget::EditorField field = static_cast<EventWidget::EditorField>(index.data(Qt::UserRole).toInt());

	// set values
	eventWidget->file()->protocol()->startNewAction("Edited "+index.data(Qt::UserRole+1).toString().toLower());
	switch(field){
		case EventWidget::MidiEventTick: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					if(spin->value() >= noteOn->offEvent()->midiTime()){
						noteOn->offEvent()->setMidiTime(spin->value()+10);
					}
				}
				event->setMidiTime(spin->value());
			}

			break;
		}
		case EventWidget::MidiEventTrack: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			MidiTrack *track = eventWidget->file()->track(box->currentIndex());
			foreach(MidiEvent *event, eventWidget->events()){
				MidiTrack *oldTrack = event->track();
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					noteOn->offEvent()->setTrack(track);
				}
				event->setTrack(track);

				TextEvent *text = qobject_cast<TextEvent*>(event);
				if(text){
					if(text->textType() == TextEvent::TrackNameTextEventType){
						oldTrack->setNameEvent(0);
						text->track()->setNameEvent(text);
					}
				}
			}
			break;
		}
		case EventWidget::MidiEventNote: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					noteOn->setNote(spin->value());
				}
				KeyPressureEvent *key = qobject_cast<KeyPressureEvent*>(event);
				if(key){
					key->setNote(spin->value());
				}
			}
			break;
		}
		case EventWidget::NoteEventOffTick: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					noteOn->offEvent()->setMidiTime(spin->value());
				}
			}
			break;
		}
		case EventWidget::NoteEventVelocity: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					noteOn->setVelocity(spin->value());
				}
			}
			break;
		}
		case EventWidget::NoteEventDuration: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent*>(event);
				if(noteOn){
					noteOn->offEvent()->setMidiTime(noteOn->midiTime()+spin->value());
				}
			}
			break;
		}
		case EventWidget::MidiEventChannel: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *ev, eventWidget->events()){
				ev->moveToChannel(spin->value());
			}
			break;
		}
		case EventWidget::MidiEventValue: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				ChannelPressureEvent *ch = qobject_cast<ChannelPressureEvent*>(event);
				if(ch){
					ch->setValue(spin->value());
				}
				ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent*>(event);
				if(ctrl){
					ctrl->setValue(spin->value());
				}
				KeyPressureEvent *key = qobject_cast<KeyPressureEvent*>(event);
				if(key){
					key->setValue(spin->value());
				}
				PitchBendEvent *pitch = qobject_cast<PitchBendEvent*>(event);
				if(pitch){
					pitch->setValue(spin->value());
				}
				TempoChangeEvent *tempo = qobject_cast<TempoChangeEvent*>(event);
				if(tempo){
					tempo->setBeats(spin->value());
				}
			}
			break;
		}
		case EventWidget::ControlChangeControl: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				ControlChangeEvent *c = qobject_cast<ControlChangeEvent*>(event);
				if(c){
					c->setControl(box->currentIndex());
				}
			}
			break;
		}
		case EventWidget::ProgramChangeProgram: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				ProgChangeEvent *c = qobject_cast<ProgChangeEvent*>(event);
				if(c){
					c->setProgram(box->currentIndex());
				}
			}
			break;
		}
		case EventWidget::KeySignatureKey: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			int tonality;
			bool minor;
			eventWidget->getKey(box->currentIndex(), &tonality, &minor);

			foreach(MidiEvent *event, eventWidget->events()){
				KeySignatureEvent *c = qobject_cast<KeySignatureEvent*>(event);
				if(c){
					c->setTonality(tonality);
					c->setMinor(minor);
				}
			}
			break;
		}
		case EventWidget::TimeSignatureNum: {
			QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				TimeSignatureEvent *ev = qobject_cast<TimeSignatureEvent*>(event);
				if(ev){
					ev->setNumerator(spin->value());
				}
			}
			break;
		}
		case EventWidget::TimeSignatureDenom: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				TimeSignatureEvent *c = qobject_cast<TimeSignatureEvent*>(event);
				if(c){
					c->setDenominator(box->currentIndex());
				}
			}
			break;
		}
		case EventWidget::TextType: {
			QComboBox *box = qobject_cast<QComboBox*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				TextEvent *c = qobject_cast<TextEvent*>(event);
				if(c){
					TextEvent::TextType oldType = c->textType();
					TextEvent::TextType newType = TextEvent::TextType(box->currentIndex()+1);
					c->setTextType(newType);
					if((oldType == TextEvent::TrackNameTextEventType) && (oldType != newType)){
						event->track()->setNameEvent(0);
					}
					if((newType == TextEvent::TrackNameTextEventType) && (oldType != newType)){
						event->track()->setNameEvent(c);
					}
				}
			}
			break;
		}
		case EventWidget::TextText: {
			QTextEdit *ed = qobject_cast<QTextEdit*>(editor);
			foreach(MidiEvent *event, eventWidget->events()){
				TextEvent *c = qobject_cast<TextEvent*>(event);
				if(c){
					c->setText(ed->toPlainText());
				}
			}
			break;
		}
		case EventWidget::UnknownType: {
			QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
			bool ok;
			int type = edit->text().toInt(&ok, 16);
			QMap<int, QString> registeredTypes = MidiEvent::knownMetaTypes();

			if(registeredTypes.keys().contains(type)){
				QMessageBox::warning(eventWidget, "Error", QString("The entered type refers to known Meta Event ("+registeredTypes.value(type)+")"));
				return;
			}

			foreach(MidiEvent *event, eventWidget->events()){
				UnknownEvent *c = qobject_cast<UnknownEvent*>(event);
				if(c){
					c->setUnknownType(type);
				}
			}
			break;
		}
		case EventWidget::MidiEventData: {
			DataEditor *edit = qobject_cast<DataEditor*>(editor);
			QByteArray data = edit->data();
			if(eventWidget->type() == MidiEvent::SystemExclusiveEventType){
				if(data.contains(quint8(0xF7))){
					QMessageBox::warning(eventWidget, "Error", QString("The data must not contain byte 0xF7 (End of SysEx)"));
					return;
				}
			}
			foreach(MidiEvent *event, eventWidget->events()){
				UnknownEvent *c = qobject_cast<UnknownEvent*>(event);
				if(c){
					c->setData(data);
				}
				SysExEvent *s = qobject_cast<SysExEvent*>(event);
				if(s){
					s->setData(data);
				}
			}

			break;
		}
	}
	eventWidget->file()->protocol()->endAction();

	// refresh model
	if(field != EventWidget::MidiEventData){
		model->setData(index, eventWidget->fieldContent(field));
	} else {
		DataEditor *edit = qobject_cast<DataEditor*>(editor);
		QByteArray data = edit->data();
		model->setData(index, data, Qt::UserRole+2);
		model->setData(index, EventWidget::dataToString(data));
	}
}

EventWidget::EventWidget(QWidget *parent) : QTableWidget(0, 2, parent) {

	_file = 0;

	QHeaderView *headerView = new QHeaderView(Qt::Horizontal, this);
	setHorizontalHeader(headerView);
	headerView->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	headerView->setSectionResizeMode(1, QHeaderView::Stretch);

	QStringList headers;
	headers.append("Property");
	headers.append("Value");
	setHorizontalHeaderLabels(headers);

	verticalHeader()->setVisible(false);

	setEditTriggers(QAbstractItemView::DoubleClicked
		| QAbstractItemView::SelectedClicked);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	setItemDelegate(new EventWidgetDelegate(this));
}

void EventWidget::setFile(MidiFile *file){
	_file = file;
	_events.clear();
	emit selectionChanged(_events.count()>0);
	reload();
}

MidiFile *EventWidget::file(){
	return _file;
}

void EventWidget::setEvents(QList<MidiEvent*> events){
	_events = events;
	emit selectionChanged(_events.count()>0);
}

QList<MidiEvent*> EventWidget::events(){
	return _events;
}

void EventWidget::removeEvent(MidiEvent *event){
	_events.removeAll(event);
	emit selectionChanged(_events.count()>0);
}

void EventWidget::reload(){

	clear();

	QStringList headers;
	headers.append("Property");
	headers.append("Value");
	setHorizontalHeaderLabels(headers);

	if(_events.isEmpty()){
		setRowCount(0);
		return;
	}

	// compute type to display
	_currentType = computeType();
	QList<QPair<QString, EditorField> > fields = getFields();
	setRowCount(fields.size()+1);

	int row = 0;

	// display type
	QTableWidgetItem *typeLabel = new QTableWidgetItem("Type");
	typeLabel->setFlags(Qt::ItemIsEnabled);
	QTableWidgetItem *type = new QTableWidgetItem(eventType());
	type->setFlags(Qt::ItemIsEnabled);
	setItem(row, 0, typeLabel);
	setItem(row++, 1, type);

	typedef QPair<QString, EditorField> FieldPair;
	foreach(FieldPair pair, fields){

		QTableWidgetItem *label = new QTableWidgetItem(pair.first);
		label->setFlags(Qt::ItemIsEnabled);
		//label->setTextAlignment(Qt::Align);
		setItem(row, 0, label);

		QTableWidgetItem *value = new QTableWidgetItem;
		value->setData(0, fieldContent(pair.second));
		value->setData(Qt::UserRole, QVariant(pair.second));
		value->setData(Qt::UserRole+1, QVariant(pair.first));
		if(pair.second == MidiEventData){
			value->setData(Qt::UserRole+2, value->data(0));
			value->setData(0, dataToString(value->data(Qt::UserRole+2).toByteArray()));
		}
		setItem(row++, 1, value);
	}

	resizeRowsToContents();
}

MidiEvent::EventType EventWidget::computeType(){
	MidiEvent::EventType type = MidiEvent::MidiEventType;
	bool inited = false;
	foreach(MidiEvent *event, _events){
		if (!inited && event->type() != MidiEvent::OnEventType
				&& event->type() != MidiEvent::OffEventType) {
			type = event->type();
		} else {
			if (type != event->type())
				type = MidiEvent::MidiEventType;
		}
		inited = true;
	}
	return type;
}

QString EventWidget::eventType(){
	switch(_currentType){
		case MidiEvent::MidiEventType: {
			return "Midi Event";
		}
		case MidiEvent::ChannelPressureEventType: {
			return "Channel Pressure Event";
		}
		case MidiEvent::ControlChangeEventType: {
			return "Control Change Event";
		}
		case MidiEvent::KeyPressureEventType: {
			return "Key Pressure Event";
		}
		case MidiEvent::KeySignatureEventType: {
			return "Key Signature Event";
		}
		case MidiEvent::NoteOnEventType: {
			return "Note On/Off Event";
		}
		case MidiEvent::PitchBendEventType: {
			return "Pitch Bend Event";
		}
		case MidiEvent::ProgramChangeEventType: {
			return "Program Change Event";
		}
		case MidiEvent::SystemExclusiveEventType: {
			return "Sysex Event";
		}
		case MidiEvent::TempoChangeEventType: {
			return "Temo Change Event";
		}
		case MidiEvent::TextEventType: {
			return "Text Event";
		}
		case MidiEvent::TimeSignatureEventType: {
			return "Time Signature Event";
		}
		case MidiEvent::UnknownEventType: {
			return "Unknown Event";
		}
		default: {
			return "Midi Event";
		}
	}

	return "Midi Event";
}

QList<QPair<QString, EventWidget::EditorField> > EventWidget::getFields(){

	QList<QPair<QString, EditorField> > fields;
	fields.append(QPair<QString, EditorField>("On Tick", MidiEventTick));

	switch(_currentType){
		case MidiEvent::ChannelPressureEventType: {
			fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}
		case MidiEvent::ControlChangeEventType: {
			fields.append(QPair<QString, EditorField>("Control", ControlChangeControl));
			fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}
		case MidiEvent::KeyPressureEventType: {
			fields.append(QPair<QString, EditorField>("Note", MidiEventNote));
			fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}
		case MidiEvent::KeySignatureEventType: {
			fields.append(QPair<QString, EditorField>("Key", KeySignatureKey));
			break;
		}
		case MidiEvent::NoteOnEventType: {
			fields.append(QPair<QString, EditorField>("Off Tick", NoteEventOffTick));
			fields.append(QPair<QString, EditorField>("Duration", NoteEventDuration));
			fields.append(QPair<QString, EditorField>("Note", MidiEventNote));
			fields.append(QPair<QString, EditorField>("Velocity", NoteEventVelocity));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}

		case MidiEvent::PitchBendEventType: {
			fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}
		case MidiEvent::ProgramChangeEventType: {
			fields.append(QPair<QString, EditorField>("Program", ProgramChangeProgram));
			fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
			break;
		}
		case MidiEvent::SystemExclusiveEventType: {
			fields.append(QPair<QString, EditorField>("Data", MidiEventData));
			break;
		}
		case MidiEvent::TempoChangeEventType: {
			fields.append(QPair<QString, EditorField>("Tempo", MidiEventValue));
			break;
		}
		case MidiEvent::TextEventType: {
			fields.append(QPair<QString, EditorField>("Type", TextType));
			fields.append(QPair<QString, EditorField>("Text", TextText));
			break;
		}
		case MidiEvent::TimeSignatureEventType: {
			fields.append(QPair<QString, EditorField>("Numerator", TimeSignatureNum));
			fields.append(QPair<QString, EditorField>("Denominator", TimeSignatureDenom));
			break;
		}
		case MidiEvent::UnknownEventType: {
			fields.append(QPair<QString, EditorField>("Type", UnknownType));
			fields.append(QPair<QString, EditorField>("Data", MidiEventData));
			break;
		}
		default: {
			// Shut up, clang.
			break;
		}
	}
	fields.append(QPair<QString, EditorField>("Track", MidiEventTrack));
	return fields;
}

QVariant EventWidget::fieldContent(EditorField field){
	switch(field){
		case MidiEventTick: {
			int tick = -1;
			foreach(MidiEvent *event, _events){
				if(tick == -1){
					tick = event->midiTime();
				} else if(tick != event->midiTime()){
					return QVariant("");
				}
			}
			return QVariant(tick);
		}
		case MidiEventTrack: {
			MidiTrack *track = 0;
			foreach(MidiEvent *event, _events){
				if(!track){
					track = event->track();
				} else if(track != event->track()){
					return QVariant("");
				}
			}
			if(!track){
				return QVariant("");
			}
			return QVariant("Track "+QString::number(track->number())+": "+track->name());
		}
		case MidiEventNote: {
			int note = -1;
			foreach(MidiEvent *event, _events){
				NoteOnEvent *onEvent = qobject_cast<NoteOnEvent*>(event);
				if(onEvent){
					if(note == -1){
						note = onEvent->note();
					} else if(note != onEvent->note()){
						return QVariant("");
					}
				}
				KeyPressureEvent *key = qobject_cast<KeyPressureEvent*>(event);
				if(key){
					if(note == -1){
						note = key->note();
					} else if(note != key->note()){
						return QVariant("");
					}
				}
			}
			if(note<0){
				return QVariant("");
			}
			return QVariant(note);
		}
		case NoteEventOffTick: {
			int off = -1;
			foreach(MidiEvent *event, _events){
				NoteOnEvent *onEvent = qobject_cast<NoteOnEvent*>(event);
				if(!onEvent){ continue; }
				if(off == -1){
					off = onEvent->offEvent()->midiTime();
				} else if(off != onEvent->offEvent()->midiTime()){
					return QVariant("");
				}
			}
			if(off<0){
				return QVariant("");
			}
			return QVariant(off);
		}
		case NoteEventVelocity: {
			int velocity = -1;
			foreach(MidiEvent *event, _events){
				NoteOnEvent *onEvent = qobject_cast<NoteOnEvent*>(event);
				if(!onEvent){ continue; }
				if(velocity == -1){
					velocity = onEvent->velocity();
				} else if(velocity != onEvent->velocity()){
					return QVariant("");
				}
			}
			if(velocity<0){
				return QVariant("");
			}
			return QVariant(velocity);
		}
		case NoteEventDuration: {
			int duration = -1;
			foreach(MidiEvent *event, _events){
				NoteOnEvent *onEvent = qobject_cast<NoteOnEvent*>(event);
				if(!onEvent){ continue; }
				if(duration == -1){
					duration = onEvent->offEvent()->midiTime()-onEvent->midiTime();
				} else if(duration != onEvent->offEvent()->midiTime()-onEvent->midiTime()){
					return QVariant("");
				}
			}
			if(duration<0){
				return QVariant("");
			}
			return QVariant(duration);
		}
		case MidiEventChannel: {
			int channel = -1;
			foreach(MidiEvent *event, _events){
				if(channel == -1){
					channel = event->channel();
				} else if(channel != event->channel()){
					return QVariant("");
				}
			}
			if(channel<0){
				return QVariant("");
			}
			return QVariant(channel);
		}

		case MidiEventValue: {
			int value = -1;
			foreach(MidiEvent *event, _events){
				{
					ChannelPressureEvent *ev = qobject_cast<ChannelPressureEvent*>(event);
					if(ev){
						if(value == -1){
							value = ev->value();
						} else if(value != ev->value()){
							return QVariant("");
						}
					}
				}
				{
					ControlChangeEvent *ev = qobject_cast<ControlChangeEvent*>(event);
					if(ev){
						if(value == -1){
							value = ev->value();
						} else if(value != ev->value()){
							return QVariant("");
						}
					}
				}
				{
					KeyPressureEvent *ev = qobject_cast<KeyPressureEvent*>(event);
					if(ev){
						if(value == -1){
							value = ev->value();
						} else if(value != ev->value()){
							return QVariant("");
						}
					}
				}
				{
					PitchBendEvent *ev = qobject_cast<PitchBendEvent*>(event);
					if(ev){
						if(value == -1){
							value = ev->value();
						} else if(value != ev->value()){
							return QVariant("");
						}
					}
				}
				{
					TempoChangeEvent *ev = qobject_cast<TempoChangeEvent*>(event);
					if(ev){
						if(value == -1){
							value = ev->beatsPerQuarter();
						} else if(value != ev->beatsPerQuarter()){
							return QVariant("");
						}
					}
				}
			}
			if(value<0){
				return QVariant("");
			}
			return QVariant(value);
		}
		case ControlChangeControl: {
			int control = -1;
			foreach(MidiEvent *event, _events){
				ControlChangeEvent *ev = qobject_cast<ControlChangeEvent*>(event);
				if(ev){
					if(control == -1){
						control = ev->control();
					} else if(control != ev->control()){
						return QVariant("");
					}
				}
			}
			if(control<0){
				return QVariant("");
			}
			return QVariant(QString::number(control)+": "+MidiFile::controlChangeName(control));
		}
		case ProgramChangeProgram: {
			int program = -1;
			foreach(MidiEvent *event, _events){
				ProgChangeEvent *ev = qobject_cast<ProgChangeEvent*>(event);
				if(ev){
					if(program == -1){
						program = ev->program();
					} else if(program != ev->program()){
						return QVariant("");
					}
				}
			}
			if(program<0){
				return QVariant("");
			}
			return QVariant(QString::number(program)+": "+MidiFile::instrumentName(program));
		}
		case KeySignatureKey: {
			int key = -1;
			foreach(MidiEvent *event, _events){
				KeySignatureEvent *ev = qobject_cast<KeySignatureEvent*>(event);
				if(ev){
					if(key == -1){
						key = keyIndex(ev->tonality(), ev->minor());
					} else if(key != keyIndex(ev->tonality(), ev->minor())){
						return QVariant("");
					}
				}
			}
			if(key<0){
				return QVariant("");
			}
			return QVariant(keyStrings().at(key));
		}
		case TimeSignatureNum: {
			int n = -1;
			foreach(MidiEvent *event, _events){
				TimeSignatureEvent *ev = qobject_cast<TimeSignatureEvent*>(event);
				if(ev){
					if(n == -1){
						n = ev->num();
					} else if(n != ev->num()){
						return QVariant("");
					}
				}
			}
			if(n<0){
				return QVariant("");
			}
			return QVariant(n);
		}
		case TimeSignatureDenom: {
			int n = -1;
			foreach(MidiEvent *event, _events){
				TimeSignatureEvent *ev = qobject_cast<TimeSignatureEvent*>(event);
				if(ev){
					if(n == -1){
						n = ev->denom();
					} else if(n != ev->denom()){
						return QVariant("");
					}
				}
			}
			if(n<0){
				return QVariant("");
			}
			return QVariant(int(qPow(2, n)));
		}
		case TextType: {
			int n = -1;
			foreach(MidiEvent *event, _events){
				TextEvent *ev = qobject_cast<TextEvent*>(event);
				if(ev){
					if(n == -1){
						n = ev->textType();
					} else if(n != ev->textType()){
						return QVariant("");
					}
				}
			}
			if(n<0){
				return QVariant("");
			}
			return QVariant(TextEvent::textTypeString(n));
		}
		case TextText: {
			bool inited = false;
			QString n = "";
			foreach(MidiEvent *event, _events){
				TextEvent *ev = qobject_cast<TextEvent*>(event);
				if(ev){
					if(!inited){
						n = ev->text();
					} else if(n.compare(ev->text())){
						return QVariant("");
					}
				}
				inited = true;
			}
			if(!inited){
				return QVariant("");
			}
			return QVariant(n);
		}
		case UnknownType: {
			int n = -1;
			foreach(MidiEvent *event, _events){
				UnknownEvent *ev = qobject_cast<UnknownEvent*>(event);
				if(ev){
					if(n==-1){
						n = ev->unknownType();
					} else if(n!=ev->unknownType()){
						return QVariant("");
					}
				}
			}
			if(n<0){
				return QVariant("");
			}
			QString s;
			s.sprintf("%02X", n);
			s="0x"+s;
			return QVariant(s);
		}
		case MidiEventData: {
			QByteArray data;
			bool inited = false;
			foreach(MidiEvent *event, _events){
				UnknownEvent *ev = qobject_cast<UnknownEvent*>(event);
				if(ev){
					if(!inited){
						data = ev->data();
					} else if(data!=ev->data()){
						return QVariant("");
					}
				}
				SysExEvent *sys = qobject_cast<SysExEvent*>(event);
				if(sys){
					if(!inited){
						data = sys->data();
					} else if(data!=sys->data()){
						return QVariant("");
					}
				}
				inited = true;
			}
			if(!inited){
				return QVariant("");
			}
			return QVariant(data);
		}
	}
	return QVariant("");
}

QStringList EventWidget::keyStrings(){
	QStringList list;
	for(int i = -6; i<=6; i++){
		list.append(KeySignatureEvent::toString(i, false));
	}
	for(int i = -6; i<=6; i++){
		list.append(KeySignatureEvent::toString(i, true));
	}
	return list;
}

int EventWidget::keyIndex(int tonality, bool minor){
	int center = 6;
	if(minor){
		center = 19;
	}
	return center+tonality;
}

void EventWidget::getKey(int index, int *tonality, bool *minor){
	if(index>13){
		*minor = true;
		index-=13;
	} else {
		*minor = false;
	}
	*tonality = index-6;
}

QString EventWidget::dataToString(QByteArray data){
	QString s;
	foreach(qint8 b, data){
		QString t;
		t.sprintf("%02X", quint8(b));
		s=s+"0x"+t+"\n";
	}
	return s.trimmed();
}

void EventWidget::reportSelectionChangedByTool(){
	emit selectionChangedByTool(_events.count()>0);
}
