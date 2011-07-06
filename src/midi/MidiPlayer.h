#ifndef MIDIPLAYER_H_
#define MIDIPLAYER_H_

#include <QObject>

class MidiFile;
class NoteOnEvent;
class PlayerThread;
class SingleNotePlayer;

class MidiPlayer : public QObject {

	Q_OBJECT

	public:
		static void play(MidiFile *file);
		static void play(NoteOnEvent *event);
		static void stop();
		static bool isPlaying();
		static int timeMs();
		static PlayerThread *playerThread();

		/**
		 * Send all Notes off to every channel.
		 */
		static void panic();

	private:
		static PlayerThread *filePlayer;
		static bool playing;
		static SingleNotePlayer *singleNotePlayer;
};

#endif
