#ifndef METRONOME_H
#define METRONOME_H

#include <QObject>
#include <QSoundEffect>

class MidiFile;

class Metronome : public QObject {

	Q_OBJECT

	public:
		void setFile(MidiFile *file);
		static Metronome *instance();
		static bool enabled();
		static void setEnabled(bool b);

	public slots:
		void measureUpdate(int measure, int tickInMeasure);
		void meterChanged(int n, int d);
		void playbackStarted();
		void playbackStopped();

	signals:
		void click();


	private:
		Metronome(QObject *parent = 0);
		static Metronome *createInstance();
		MidiFile *_file;
		void reset();
		int num, denom, lastPos, lastMeasure;
		static bool _enable;
		static QSoundEffect *clickSound;
};

#endif
