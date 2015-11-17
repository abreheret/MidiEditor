#ifndef METRONOME_H
#define METRONOME_H

#include <QObject>
#include <phonon/phonon>

class MidiFile;

class Metronome : public QObject {
	Q_OBJECT

	public:
		Metronome(QObject *parent = 0);
		void setFile(MidiFile *file);
		static Metronome *instance();

		static bool enabled();
		static void setEnabled(bool b);

	public slots:
		void measureUpdate(int measure, int tickInMeasure);
		void meterChanged(int n, int d);
		void playbackStarted();
		void playbackStopped();

	private:
		static Metronome *_instance;
		MidiFile *_file;
		void reset();
		int num, denom, lastPos, lastMeasure;
		void click();
		static Phonon::MediaObject *mediaObject;
		static bool _enable;
};

#endif
