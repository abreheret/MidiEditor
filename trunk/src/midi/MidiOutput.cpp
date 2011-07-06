#include "MidiOutput.h"

#include "../MidiEvent/MidiEvent.h"

#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <vector>

#include "rtmidi/RtError.h"
#include "rtmidi/RtMidi.h"

#include "SenderThread.h"

RtMidiOut *MidiOutput::_midiOut = 0;
QString MidiOutput::_outPort = "";
SenderThread *MidiOutput::_sender = new SenderThread();

void MidiOutput::init(){

	// RtMidiOut constructor
	try {
		_midiOut = new RtMidiOut();
	}
	catch ( RtError &error ) {
		qWarning(error.getMessageString());
	}
	_sender->start(QThread::TimeCriticalPriority);
}

void MidiOutput::sendCommand(QByteArray array){

	sendEnqueuedCommand(array);
}

void MidiOutput::sendCommand(MidiEvent *e){

	if(e->channel() >= 0 && e->channel() < 16){
		_sender->enqueue(e);
	}
}

QStringList MidiOutput::outputPorts(){

	QStringList ports;

	// Check outputs.
	unsigned int nPorts = _midiOut->getPortCount();

	for(unsigned int i = 0; i < nPorts; i++){

		try {
			ports.append(QString::fromStdString(_midiOut->getPortName(i)));
		}
		catch (RtError &error) {}
	}

	return ports;
}

bool MidiOutput::setOutputPort(QString name){

	// try to find the port
	unsigned int nPorts = _midiOut->getPortCount();

	for(unsigned int i = 0; i < nPorts; i++){

		try {

			// if the current port has the given name, select it and close
			// current port
			if(_midiOut->getPortName(i) == name.toStdString()){

				_midiOut->closePort();
				_midiOut->openPort(i);
				_outPort = name;
				return true;
			}

		}
		catch (RtError &error) {}

	}

	// port not found
	return false;
}

QString MidiOutput::outputPort(){
	return _outPort;
}

void MidiOutput::sendEnqueuedCommand(QByteArray array) {

	if(_outPort != ""){

		// convert data to std::vector
		std::vector<unsigned char> message;

		foreach(char byte, array){
			message.push_back(byte);
		}

		_midiOut->sendMessage(&message);
	}
}
