#ifndef SINGLENOTEPLAYER_H_
#define SINGLENOTEPLAYER_H_

#include <QObject>

class NoteOnEvent;
class QTimer;

class SingleNotePlayer : public QObject {

	Q_OBJECT

	public:
		SingleNotePlayer();
		void play(NoteOnEvent *event);

	public slots:
		void timeout();

	private:
		QTimer *timer;
		QByteArray offMessage;
		bool playing;
};

#endif
