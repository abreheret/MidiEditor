#ifndef MIDIOUTPUT_H_
#define MIDIOUTPUT_H_

#include <QObject>

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;
class SenderThread;

class MidiOutput : public QObject {

	public:
		static void init();
		static void sendCommand(QByteArray array);
		static void sendCommand(MidiEvent *e);
		static QStringList outputPorts();
		static bool setOutputPort(QString name);
		static QString outputPort();
		static void sendEnqueuedCommand(QByteArray array);

	private:
		static QString _outPort;
		static RtMidiOut *_midiOut;
		static SenderThread *_sender;
};

#endif
