#ifndef TEMPOCHANGEEVENT_H_
#define TEMPOCHANGEEVENT_H_

#include "MidiEvent.h"

#include <QSpinBox>
#include <QWidget>
#include <QLabel>

class TempoChangeEvent : public MidiEvent {

	public:
		TempoChangeEvent(int channel, int value);
		TempoChangeEvent(TempoChangeEvent &other);

		int beatsPerQuarter();
		double msPerTick();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);
		int line();
		QByteArray save();

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();

		void setBeats(int beats);

	private:
		int _beats;
		static QSpinBox *_val_box;
		static QLabel *_val_label;
		static QWidget *_val_widget;
};

#endif
