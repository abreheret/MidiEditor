#ifndef EVENTWIDGET_H_
#define EVENTWIDGET_H_

#include <QWidget>

class MidiEvent;

class EventWidget : public QWidget {

	Q_OBJECT

	public:
		EventWidget(QWidget *parent = 0);

		void setEvent(MidiEvent *event);
		MidiEvent *event();

		void reload();

	public slots:
		void submit();

	private:
		MidiEvent *_event;
		QWidget *_central;
};

#endif
