#ifndef ONEVENT_H_
#define ONEVENT_H_

#include "MidiEvent.h"
#include <QWidget>
#include <QLabel>
#include <QSpinBox>
class OffEvent;

class OnEvent : public MidiEvent{

	public:
		OnEvent(int ch);
		OnEvent(OnEvent &other);

		void setOffEvent(OffEvent *event);
		OffEvent *offEvent();
		virtual QByteArray saveOffEvent();
		virtual QString offEventMessage();
		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		void generateWidget(QWidget *widget);
		void editByWidget();

	protected:
		OffEvent *_offEvent;
		static QWidget *_off_tick_widget;
		static QSpinBox *_off_tick_box;
		static QLabel *_off_tick_label;

};

#endif
