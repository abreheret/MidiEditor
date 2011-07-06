#ifndef PROTOCOLWIDGET_H_
#define PROTOCOLWIDGET_H_

#include "PaintWidget.h"
#include <QPaintEvent>

class MidiFile;

class ProtocolWidget : public PaintWidget {

	Q_OBJECT

	public:
		ProtocolWidget(QWidget *parent = 0);
		void setFile(MidiFile *f);

	public slots:
		void protocolChanged();

	private:
		MidiFile *file;

	protected:
		void paintEvent(QPaintEvent *event);
};

#endif
