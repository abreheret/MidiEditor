#ifndef PLAYERTHREAD_H_
#define PLAYERTHREAD_H_

#include <QThread>
#include <QObject>
#include <QMultiMap>
#include <QTimer>

class MidiFile;
class MidiEvent;

class PlayerThread : public QThread {

	Q_OBJECT

	public:
		PlayerThread();
		void setFile(MidiFile *f);
		void stop();
		void run();
		void setInterval(int i);
		int timeMs();

	public slots:
		void timeout();

	signals:
		void timeMsChanged(int ms);
		void playerStopped();

	private:
		MidiFile *file;
		QMultiMap<int, MidiEvent*> *events;
		int interval, position, timeoutSinceLastSignal;
		volatile bool stopped;
		QTimer *timer;
};

#endif
