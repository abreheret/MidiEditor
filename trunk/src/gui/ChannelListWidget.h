#ifndef CHANNELLISTWIDGET_H_
#define CHANNELLISTWIDGET_H_

#include "PaintWidget.h"
#include <QPaintEvent>

class MidiFile;

class ChannelListWidget : public PaintWidget {

	Q_OBJECT

	public:
		ChannelListWidget(QWidget *parent = 0);
		void setFile(MidiFile *f);
	signals:
		void channelStateChanged();

	private:
		MidiFile *file;

	protected:
		void paintEvent(QPaintEvent *event);
};

#endif
