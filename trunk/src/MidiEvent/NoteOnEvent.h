#ifndef NOTEONEVENT_H_
#define NOTEONEVENT_H_

#include "OnEvent.h"
#include <QWidget>
#include <QLabel>
#include <QSpinBox>

class NoteOffEvent;

class NoteOnEvent : public OnEvent{

	public:
		NoteOnEvent(int note, int velocity, int ch);
		NoteOnEvent(NoteOnEvent &other);

		int note();
		int velocity();
		int line();

		void setNote(int n);
		void setVelocity(int v);
		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);
		QString toMessage();
		QString offEventMessage();
		QByteArray save();
		QByteArray saveOffEvent();

		void generateWidget(QWidget *widget);
		void editByWidget();
		QString typeString();

	protected:
		int _note, _velocity;

		static QWidget *_note_widget, *_velocity_widget;
		static QSpinBox *_note_box, *_velocity_box;
		static QLabel *_note_label, *_velocity_label;
};

#endif
