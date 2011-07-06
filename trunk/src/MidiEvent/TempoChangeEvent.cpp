#include "TempoChangeEvent.h"
#include "../midi/MidiFile.h"

#include <QSpinBox>
#include <QWidget>
#include <QLabel>
#include <QLayout>

TempoChangeEvent::TempoChangeEvent(int channel, int value) : MidiEvent(channel){
	_beats = 60000000/value;
}

TempoChangeEvent::TempoChangeEvent(TempoChangeEvent &other) : MidiEvent(other){
	_beats = other._beats;
}

int TempoChangeEvent::beatsPerQuarter(){
	return _beats;
}

double TempoChangeEvent::msPerTick(){
	double quarters_per_second = (double)_beats/60;
	double ticks_per_second = (double)(file()->ticksPerQuarter()) *
			quarters_per_second;
	return 1000/(ticks_per_second);
}

ProtocolEntry *TempoChangeEvent::copy(){
	return new TempoChangeEvent(*this);
}

void TempoChangeEvent::reloadState(ProtocolEntry *entry){
	TempoChangeEvent *other = dynamic_cast<TempoChangeEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_beats = other->_beats;
}

int TempoChangeEvent::line(){
	return MidiEvent::TEMPO_CHANGE_EVENT_LINE;
}

QByteArray TempoChangeEvent::save(){
	QByteArray array = QByteArray();

	array.append(0xFF);
	array.append(0x51);
	array.append(0x03);
	int value = 60000000/_beats;
	for(int i = 2; i >=0; i--){
		array.append((value & (0xFF << 8*i)) >>8*i);
	}

	return array;
}

void TempoChangeEvent::setBeats(int beats){
	ProtocolEntry *toCopy = copy();
	_beats = beats;
	file()->calcMaxTime();
	protocol(toCopy, this);

}

QString TempoChangeEvent::typeString(){
	return "Tempo Change Event";
}

// Widgets for EventWidget
QWidget *TempoChangeEvent::_val_widget = 0;
QSpinBox *TempoChangeEvent::_val_box = 0;
QLabel *TempoChangeEvent::_val_label = 0;

void TempoChangeEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_val_widget == 0) _val_widget = new QWidget();
	if(_val_box == 0) _val_box = new QSpinBox();
	if(_val_label == 0) _val_label = new QLabel();

	// set Parents
	_val_widget->setParent(widget);
	_val_label->setParent(_val_widget);
	_val_box->setParent(_val_widget);

	// unhide
	_val_widget->setHidden(false);
	_val_label->setHidden(false);
	_val_box->setHidden(false);

	// Edit value
	_val_label->setText("Beats per Minute: ");
	QLayout *valL = _val_widget->layout();
	if(!valL){
		valL = new QBoxLayout(QBoxLayout::LeftToRight, _val_widget);
		_val_widget->setLayout(valL);
	}
	valL->addWidget(_val_label);

	// box
	_val_box->setMinimum(1);
	_val_box->setMaximum(1000);
	_val_box->setValue(beatsPerQuarter());
	valL->addWidget(_val_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_val_widget);
}

void TempoChangeEvent::editByWidget(){
	if(beatsPerQuarter()!=_val_box->value()){
		setBeats(_val_box->value());
	}
	MidiEvent::editByWidget();
}
