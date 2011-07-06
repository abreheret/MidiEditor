#include "UnknownEvent.h"

#include <QLabel>
#include <QLayout>

UnknownEvent::UnknownEvent(int channel, QByteArray data) : MidiEvent(channel){
	_data = data;
}

QByteArray UnknownEvent::data(){
	return _data;
}

int UnknownEvent::line(){
	return UNKNOWN_LINE;
}

QByteArray UnknownEvent::save(){
	return data();
}

void UnknownEvent::generateWidget(QWidget *widget){
	MidiEvent::generateWidget(widget);
	QLayout *layout = widget->layout();
	int i = 0;
	foreach(char b,_data){
		QLabel *l = new QLabel("0x"+QString::number(i, 16)+"   "+b);
		layout->addWidget(l);
		i++;
	}
}
