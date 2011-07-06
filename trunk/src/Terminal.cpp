#include "Terminal.h"

#include <QTextEdit>
#include <QProcess>
#include <QScrollBar>
#include <QTimer>

#include "midi/MidiInput.h"
#include "midi/MidiOutput.h"

Terminal *Terminal::_terminal = 0;

Terminal::Terminal() {
	_process = 0;
	_textEdit = new QTextEdit();
	_textEdit->setReadOnly(true);

	_inPort = "";
	_outPort = "";
}

void Terminal::initTerminal(QString startString, QString inPort,
		QString outPort)
{
	_terminal = new Terminal();
	_terminal->execute(startString, inPort, outPort);
}

Terminal *Terminal::terminal(){
	return _terminal;
}

void Terminal::writeString(QString message){
	_textEdit->setText(_textEdit->toPlainText()+message+"\n");
	_textEdit->verticalScrollBar()->setValue(
			_textEdit->verticalScrollBar()->maximum());
}

void Terminal::execute(QString startString, QString inPort,
	QString outPort)
{
	_inPort = inPort;
	_outPort = outPort;

	if(startString!=""){
		if(_process){
			_process->kill();
		}
		_process = new QProcess();

		connect(_process, SIGNAL(readyReadStandardOutput()),
			this, SLOT(printToTerminal()));
		connect(_process, SIGNAL(readyReadStandardError()),
			this, SLOT(printErrorToTerminal()));
		connect(_process, SIGNAL(started()),
			this, SLOT(processStarted()));

		_process->start(startString);
	} else {
		processStarted();
	}
}

void Terminal::processStarted(){
	writeString("Started process");
	if(MidiInput::inputPort() == "" && _inPort != ""){
		writeString("Trying to set Input Port to "+_inPort);
		foreach(QString port, MidiInput::inputPorts()){
			if(port.startsWith(_inPort)){
				writeString("Found port "+port);
				MidiInput::setInputPort(port);
				_inPort = "";
				break;
			}
		}
	}
	if(MidiOutput::outputPort()== "" && _outPort != ""){
		writeString("Trying to set Output Port to "+_outPort);
		foreach(QString port, MidiOutput::outputPorts()){
			if(port.startsWith(_outPort)){
				writeString("Found port "+port);
				MidiOutput::setOutputPort(port);
				_outPort = "";
				break;
			}
		}
	}

	// if not both are set, try again in 1 second
	if((MidiOutput::outputPort()== "" && _outPort != "") ||
			(MidiInput::inputPort() == "" && _inPort != "")){
		QTimer *timer = new QTimer();
		connect(timer, SIGNAL(timeout()), this, SLOT(processStarted()));
		timer->setSingleShot(true);
		timer->start(1000);
	}
}

void Terminal::printToTerminal(){
	writeString(QString::fromLocal8Bit(_process->readAllStandardOutput()));
}

void Terminal::printErrorToTerminal(){
	writeString(QString::fromLocal8Bit(_process->readAllStandardError()));
}

QTextEdit *Terminal::console(){
	return _textEdit;
}
