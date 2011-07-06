#ifndef TIMESIGNATUREEVENT_H_
#define TIMESIGNATUREEVENT_H_

#include "MidiEvent.h"

class QWidget;
class QLabel;
class QSpinBox;
class QComboBox;

class TimeSignatureEvent : public MidiEvent {

	public:
		TimeSignatureEvent(int channel, int num, int denom, int midiClocks,
				int num32In4);
		TimeSignatureEvent(TimeSignatureEvent &other);
		int num();
		int denom();
		int midiClocks();
		int num32In4();
		int measures(int tick);
		int ticksPerMeasure();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);
		int line();
		QByteArray save();

		void setDenominator(int d);
		void setNumerator(int n);

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();

	private:
		int numerator, denominator, midiClocksPerMetronome, num32In4th;

		static QSpinBox *_num_box;
		static QLabel *_num_label, *_denom_label;
		static QWidget *_num_widget, *_denom_widget;
		static QComboBox *_denom_combo;
};

#endif
