#ifndef MIDIINPUT_H_
#define MIDIINPUT_H_

#include <QObject>
#include <QProcess>
#include <QList>
#include <QMultiMap>

#include <vector>

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;

class MidiInput : public QObject {

	public:

		static void init();

		static void sendCommand(QByteArray array);
		static void sendCommand(MidiEvent *e);

		static QStringList inputPorts();
		static bool setInputPort(QString name);
		static QString inputPort();

		static void startInput();
		static QMultiMap<int, MidiEvent*> endInput();

		static void receiveMessage(double deltatime,
				std::vector< unsigned char > *message, void *userData = 0);

		static void setTime(int ms);

		static bool recording();

	private:
		static QString _inPort;
		static RtMidiIn *_midiIn;
		static QMultiMap<int, std::vector<unsigned char> > *_messages;
		static int _currentTime;
		static bool _recording;
};

#endif
