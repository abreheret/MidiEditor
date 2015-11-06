/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.+
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"

#include <QGridLayout>
#include <QProcess>

#include "MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "ChannelListWidget.h"
#include "TrackListWidget.h"
#include "../tool/Tool.h"
#include "../tool/SelectTool.h"
#include "../tool/SizeChangeTool.h"
#include "../tool/EraserTool.h"
#include "../tool/StandardTool.h"
#include "../tool/NewNoteTool.h"
#include "../tool/EventMoveTool.h"
#include "../tool/EventTool.h"
#include "EventWidget.h"
#include "../tool/ToolButton.h"
#include <QScrollArea>
#include "../protocol/Protocol.h"
#include "ProtocolWidget.h"
#include "ClickButton.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
#include "InstrumentChooser.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QSplitter>
#include <QMessageBox>
#include <QTabWidget>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QIcon>
#include <QStringList>
#include <QComboBox>
#include <QToolButton>

#include "../midi/MidiChannel.h"
#include "DonateDialog.h"
#include "AboutDialog.h"
#include "FileLengthDialog.h"
#include <QTextStream>
#include "../midi/MidiOutput.h"
#include <QInputDialog>
#include "MidiSettingsDialog.h"
#include "../midi/MidiInput.h"
#include <QMultiMap>
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "RecordDialog.h"
#include <QSettings>
#include "../Terminal.h"
#include <QTextEdit>
#include <QToolBar>
#include <QInputDialog>
#include "../MidiEvent/OnEvent.h"
#include "TransposeDialog.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../remote/RemoteServer.h"
#include "RemoteDialog.h"
#include "MiscWidget.h"

MainWindow::MainWindow() : QMainWindow() {

	file = 0;
	_settings = new QSettings(QString("MidiEditor"), QString("NONE"));

	bool ok;
	int port = _settings->value("udp_client_port", -1).toInt(&ok);
	QString ip = _settings->value("udp_client_ip", "").toString();


	 _remoteServer = new RemoteServer();
	 _remoteServer->setIp(ip);
	 _remoteServer->setPort(port);
	 _remoteServer->tryConnect();


	connect(_remoteServer, SIGNAL(playRequest()), this, SLOT(play()));
	connect(_remoteServer, SIGNAL(stopRequest(bool, bool)), this, SLOT(stop(bool, bool)));
	connect(_remoteServer, SIGNAL(recordRequest()), this, SLOT(record()));
	connect(_remoteServer, SIGNAL(backRequest()), this, SLOT(back()));
	connect(_remoteServer, SIGNAL(forwardRequest()), this, SLOT(forward()));
	connect(_remoteServer, SIGNAL(pauseRequest()), this, SLOT(pause()));

	connect(MidiPlayer::playerThread(),
			SIGNAL(timeMsChanged(int)), _remoteServer, SLOT(setTime(int)));
	connect(MidiPlayer::playerThread(),
			SIGNAL(meterChanged(int, int)), _remoteServer, SLOT(setMeter(int, int)));
	connect(MidiPlayer::playerThread(),
			SIGNAL(tonalityChanged(int)), _remoteServer, SLOT(setTonality(int)));
	connect(MidiPlayer::playerThread(),
			SIGNAL(measureChanged(int)), _remoteServer, SLOT(setMeasure(int)));


	startDirectory = QDir::homePath();

	if(_settings->value("open_path").toString()!=""){
		startDirectory = _settings->value("open_path").toString();
	} else {
		_settings->setValue("open_path", startDirectory);
	}

	// read recent paths
	_recentFilePaths = _settings->value("recent_file_list").toStringList();

	EditorTool::setMainWindow(this);

	setWindowTitle("MidiEditor 2.5.0");
	setWindowIcon(QIcon("graphics/icon.png"));

	QWidget *central = new QWidget(this);
	QGridLayout *centralLayout = new QGridLayout(central);
	centralLayout->setContentsMargins(3,3,3,5);

	// there is a vertical split
	QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, central);
	//mainSplitter->setHandleWidth(0);

	// The left side
	QSplitter *leftSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	leftSplitter->setHandleWidth(0);
	mainSplitter->addWidget(leftSplitter);
	leftSplitter->setContentsMargins(0,0,0,0);

	// The right side
	QSplitter *rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	//rightSplitter->setHandleWidth(0);
	mainSplitter->addWidget(rightSplitter);

	// Set the sizes of mainSplitter
	mainSplitter->setStretchFactor(0, 1);
	mainSplitter->setStretchFactor(1, 0);
	mainSplitter->setContentsMargins(0,0,0,0);

	// the channelWidget and the trackWidget are tabbed
	QTabWidget *upperTabWidget = new QTabWidget(rightSplitter);
	rightSplitter->addWidget(upperTabWidget);
	rightSplitter->setContentsMargins(0,0,0,0);

	// protocolList and EventWidget are tabbed
	lowerTabWidget = new QTabWidget(rightSplitter);
	rightSplitter->addWidget(lowerTabWidget);

	// MatrixArea
	QWidget *matrixArea = new QWidget(leftSplitter);
	leftSplitter->addWidget(matrixArea);
	mw_matrixWidget = new MatrixWidget(matrixArea);
	vert = new QScrollBar(Qt::Vertical, matrixArea);
	QGridLayout *matrixAreaLayout = new QGridLayout(matrixArea);
    QWidget *placeholder0 = new QWidget(matrixArea);
    placeholder0->setFixedHeight(50);
	matrixAreaLayout->setContentsMargins(0,0,0,0);
    matrixAreaLayout->addWidget(mw_matrixWidget, 0, 0, 2, 1);
    matrixAreaLayout->addWidget(placeholder0, 0, 1, 1, 1);
    matrixAreaLayout->addWidget(vert, 1, 1, 1, 1);
	matrixAreaLayout->setColumnStretch(0, 1);
	matrixArea->setLayout(matrixAreaLayout);

	// VelocityArea
	QWidget *velocityArea = new QWidget(leftSplitter);
	leftSplitter->addWidget(velocityArea);
	hori = new QScrollBar(Qt::Horizontal, velocityArea);
	hori->setSingleStep(500);
	QGridLayout *velocityAreaLayout = new QGridLayout(velocityArea);
    velocityAreaLayout->setContentsMargins(0,0,0,0);
    _miscWidgetControl = new QWidget(velocityArea);
    _miscWidgetControl->setFixedWidth(110-velocityAreaLayout->horizontalSpacing());
    velocityAreaLayout->addWidget(_miscWidgetControl, 0,0,1,1);
	// there is a Scrollbar on the right side of the velocityWidget doing
	// nothing but making the VelocityWidget as big as the matrixWidget
	QScrollBar *scrollNothing = new QScrollBar(Qt::Vertical, velocityArea);
	scrollNothing->setMinimum(0);
	scrollNothing->setMaximum(0);
    velocityAreaLayout->addWidget(scrollNothing, 0,2,1,1);
    velocityAreaLayout->addWidget(hori, 1, 1, 1, 1);
	velocityAreaLayout->setRowStretch(0, 1);
	velocityArea->setLayout(velocityAreaLayout);

    _miscWidget = new MiscWidget(mw_matrixWidget, velocityArea);
    velocityAreaLayout->addWidget(_miscWidget, 0, 1, 1, 1);

    // controls for velocity widget
    _miscControlLayout = new QGridLayout(_miscWidgetControl);
    _miscControlLayout->setHorizontalSpacing(0);
    _miscWidgetControl->setContentsMargins(0,0,0,0);
    _miscWidgetControl->setLayout(_miscControlLayout);
    _miscMode = new QComboBox(_miscWidgetControl);
    for(int i = 0; i<MiscModeEnd; i++){
        _miscMode->addItem(MiscWidget::modeToString(i));
    }
    //_miscControlLayout->addWidget(new QLabel("Mode:", _miscWidgetControl), 0, 0, 1, 3);
    _miscControlLayout->addWidget(_miscMode, 1, 0, 1, 3);
    connect(_miscMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMiscMode(int)));

    //_miscControlLayout->addWidget(new QLabel("Control:", _miscWidgetControl), 2, 0, 1, 3);
    _miscController = new QComboBox(_miscWidgetControl);
    for(int i = 0; i<128; i++){
        _miscController->addItem(MidiFile::controlChangeName(i));
    }
    _miscControlLayout->addWidget(_miscController, 3, 0, 1, 3);
    connect(_miscController, SIGNAL(currentIndexChanged(int)), _miscWidget, SLOT(setControl(int)));

    //_miscControlLayout->addWidget(new QLabel("Channel:", _miscWidgetControl), 4, 0, 1, 3);
    _miscChannel = new QComboBox(_miscWidgetControl);
    for(int i = 0; i<15; i++){
        _miscChannel->addItem("Channel "+QString::number(i));
    }
    _miscControlLayout->addWidget(_miscChannel, 5, 0, 1, 3);
    connect(_miscChannel, SIGNAL(currentIndexChanged(int)), _miscWidget, SLOT(setChannel(int)));

    _miscMode->setCurrentIndex(0);
    _miscChannel->setEnabled(false);
    _miscController->setEnabled(false);

    setSingleMode = new QAction(QIcon("graphics/tool/misc_single.png"), "Select Single", this);
    setSingleMode->setCheckable(true);
    setFreehandMode = new QAction(QIcon("graphics/tool/misc_freehand.png"), "Free-hand", this);
    setFreehandMode->setCheckable(true);
    setLineMode = new QAction(QIcon("graphics/tool/misc_line.png"),"Line Mode", this);
    setLineMode->setCheckable(true);

    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    group->addAction(setSingleMode);
    group->addAction(setFreehandMode);
    group->addAction(setLineMode);
    setSingleMode->setChecked(true);
    connect(group, SIGNAL(triggered(QAction*)), this, SLOT(selectModeChanged(QAction*)));

    QToolButton *btnSingle = new QToolButton(_miscWidgetControl);
    btnSingle->setDefaultAction(setSingleMode);
    QToolButton *btnHand = new QToolButton(_miscWidgetControl);
    btnHand->setDefaultAction(setFreehandMode);
    QToolButton *btnLine = new QToolButton(_miscWidgetControl);
    btnLine->setDefaultAction(setLineMode);

    _miscControlLayout->addWidget(btnSingle, 9, 0, 1, 1);
    _miscControlLayout->addWidget(btnHand, 9, 1, 1, 1);
    _miscControlLayout->addWidget(btnLine, 9, 2, 1, 1);

	// Set the sizes of leftSplitter
    leftSplitter->setStretchFactor(0, 8);
    leftSplitter->setStretchFactor(1, 1);

	// Channels
	QScrollArea *channelScroll = new QScrollArea(upperTabWidget);
	channelWidget = new ChannelListWidget(channelScroll);
	connect(channelWidget, SIGNAL(channelStateChanged()), this, SLOT(updateChannelMenu()), Qt::QueuedConnection);
	connect(channelWidget, SIGNAL(selectInstrumentClicked(int)), this, SLOT(setInstrumentForChannel(int)), Qt::QueuedConnection);
	channelScroll->setWidget(channelWidget);
	channelScroll->setWidgetResizable(true);
	upperTabWidget->addTab(channelScroll, "Channels");

	// Track
	QWidget *trackWidget = new QWidget(upperTabWidget);
	QGridLayout *trackLayout = new QGridLayout(trackWidget);
	trackWidget->setLayout(trackLayout);

	QScrollArea *trackScroll = new QScrollArea(trackWidget);
	_trackWidget = new TrackListWidget(trackScroll);
	connect(_trackWidget, SIGNAL(trackRenameClicked(int)), this, SLOT(renameTrack(int)), Qt::QueuedConnection);
	connect(_trackWidget, SIGNAL(trackRemoveClicked(int)), this, SLOT(removeTrack(int)), Qt::QueuedConnection);
	trackScroll->setWidget(_trackWidget);
	trackLayout->addWidget(trackScroll, 0, 0, 1, 2);

	QPushButton *addTrackButton = new QPushButton("Add Track", trackWidget);
	connect(addTrackButton, SIGNAL(clicked()), this, SLOT(addTrack()));
	trackLayout->addWidget(addTrackButton, 1, 1, 1, 1);

	trackScroll->setWidgetResizable(true);
	upperTabWidget->addTab(trackWidget, "Tracks");

	// terminal
	Terminal::initTerminal(_settings->value("start_cmd", "").toString(),
			_settings->value("in_port", "").toString(),
			_settings->value("out_port", "").toString());
	upperTabWidget->addTab(Terminal::terminal()->console(), "Terminal");

	// Protocollist
	QScrollArea *protocolScroll = new QScrollArea(lowerTabWidget);
	protocolWidget = new ProtocolWidget(protocolScroll);
	protocolScroll->setWidget(protocolWidget);
	protocolScroll->setWidgetResizable(true);
	lowerTabWidget->addTab(protocolScroll, "Protocol");

	// EventWidget
	QScrollArea *eventScroll = new QScrollArea(lowerTabWidget);
	_eventWidget = new EventWidget(eventScroll);
	eventScroll->setWidget(_eventWidget);
	eventScroll->setWidgetResizable(true);
	lowerTabWidget->addTab(eventScroll, "Event");
	MidiEvent::setEventWidget(_eventWidget);
	connect(_eventWidget, SIGNAL(eventSelected(MidiEvent*)), this, SLOT(showEventWidget(MidiEvent*)));

	// below add two rows for choosing track/channel new events shall be assigned to
	QWidget *chooser = new QWidget(rightSplitter);
	chooser->setMinimumWidth(350);
	rightSplitter->addWidget(chooser);
	QGridLayout *chooserLayout = new QGridLayout(chooser);
	QLabel *trackchannelLabel = new QLabel("Choose Track and Channel for new Events");
	chooserLayout->addWidget(trackchannelLabel, 0,0,1, 2);
	QLabel *channelLabel = new QLabel("Channel: ", chooser);
	chooserLayout->addWidget(channelLabel, 1, 0, 1, 1);
	_chooseEditChannel = new QComboBox(chooser);
	for(int i = 0; i<16;i++){
		_chooseEditChannel->addItem("Channel "+QString::number(i));
	}
	connect(_chooseEditChannel, SIGNAL(activated(int)), this, SLOT(editChannel(int)));

	chooserLayout->addWidget(_chooseEditChannel, 1, 1, 1, 1);
	QLabel *trackLabel = new QLabel("Track: ", chooser);
	chooserLayout->addWidget(trackLabel, 2, 0, 1, 1);
	_chooseEditTrack = new QComboBox(chooser);
	chooserLayout->addWidget(_chooseEditTrack, 2, 1, 1, 1);
	connect(_chooseEditTrack, SIGNAL(activated(int)), this, SLOT(editTrack(int)));

	// connect Scrollbars and Widgets
	connect(vert, SIGNAL(valueChanged(int)), mw_matrixWidget,
			SLOT(scrollYChanged(int)));
	connect(hori, SIGNAL(valueChanged(int)), mw_matrixWidget,
			SLOT(scrollXChanged(int)));

	connect(channelWidget, SIGNAL(channelStateChanged()), mw_matrixWidget,
			SLOT(repaint()));
	connect(mw_matrixWidget, SIGNAL(sizeChanged(int, int, int, int)), this,
			SLOT(matrixSizeChanged(int, int, int, int)));

	connect(mw_matrixWidget, SIGNAL(scrollChanged(int, int, int, int)), this,
			SLOT(scrollPositionsChanged(int, int, int, int)));

	setCentralWidget(central);

	QWidget *buttons = setupActions(central);

	rightSplitter->setStretchFactor(0, 15);
	rightSplitter->setStretchFactor(1, 85);

    // Add the Widgets to the central Layout
    centralLayout->setSpacing(0);
    centralLayout->addWidget(buttons,0,0);
    centralLayout->addWidget(mainSplitter,1,0);
    centralLayout->setRowStretch(1, 1);
    central->setLayout(centralLayout);

    this->newFile();
}

void MainWindow::scrollPositionsChanged(int startMs,int maxMs,int startLine,
		int maxLine)
{
	hori->setMaximum(maxMs);
	hori->setValue(startMs);
	vert->setMaximum(maxLine);
	vert->setValue(startLine);
}

void MainWindow::setFile(MidiFile *file){

	NewNoteTool::setEditTrack(0);
	NewNoteTool::setEditChannel(0);

	protocolWidget->setFile(file);
	channelWidget->setFile(file);
	_trackWidget->setFile(file);
	_remoteServer->setFile(file);
	Tool::setFile(file);
	this->file = file;
	connect(file, SIGNAL(trackChanged()), this, SLOT(updateTrackMenu()));
	setWindowTitle("MidiEditor - " +file->path()+"[*]");
	connect(file,SIGNAL(cursorPositionChanged()),channelWidget,SLOT(update()));
	connect(file,SIGNAL(recalcWidgetSize()),mw_matrixWidget,SLOT(calcSizes()));
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(markEdited()));

	mw_matrixWidget->setFile(file);
	updateChannelMenu();
	updateTrackMenu();
}

void MainWindow::matrixSizeChanged(int maxScrollTime, int maxScrollLine,
		int vX, int vY)
{
	vert->setMaximum(maxScrollLine);
	hori->setMaximum(maxScrollTime);
	vert->setValue(vY);
	hori->setValue(vX);
	mw_matrixWidget->repaint();
}

void MainWindow::play(){

	if(file && !MidiInput::recording() && !MidiPlayer::isPlaying()){

		mw_matrixWidget->timeMsChanged(file->msOfTick(file->cursorTick()), true);

        _miscWidget->setEnabled(false);
		channelWidget->setEnabled(false);
		protocolWidget->setEnabled(false);
		mw_matrixWidget->setEnabled(false);
		_trackWidget->setEnabled(false);

	    MidiPlayer::play(file);
		connect(MidiPlayer::playerThread(),
				SIGNAL(playerStopped()), this,	SLOT(stop()));

		#ifdef __WINDOWS_MM__
		connect(MidiPlayer::playerThread(),
				SIGNAL(timeMsChanged(int)), mw_matrixWidget, SLOT(timeMsChanged(int)));
		#endif

		_remoteServer->play();
	}
}


void MainWindow::record(){

	if(!file){
		newFile();
	}

	if(!MidiInput::recording() && !MidiPlayer::isPlaying()){
		// play current file
		if(file){

			if(file->pauseTick() >= 0){
				file->setCursorTick(file->pauseTick());
				file->setPauseTick(-1);
			}

			mw_matrixWidget->timeMsChanged(file->msOfTick(file->cursorTick()), true);

            _miscWidget->setEnabled(false);
			channelWidget->setEnabled(false);
			protocolWidget->setEnabled(false);
			mw_matrixWidget->setEnabled(false);
			_trackWidget->setEnabled(false);

			_remoteServer->record();

			MidiPlayer::play(file);
			MidiInput::startInput();
			connect(MidiPlayer::playerThread(),
					SIGNAL(playerStopped()), this,	SLOT(stop()));
			#ifdef __WINDOWS_MM__
			connect(MidiPlayer::playerThread(),
					SIGNAL(timeMsChanged(int)), mw_matrixWidget, SLOT(timeMsChanged(int)));
			#endif
		}
	}
}


void MainWindow::pause(){
	if(file){
		if(MidiPlayer::isPlaying()){
			file->setPauseTick(file->tick(MidiPlayer::timeMs()));
			stop(false, false, false);
		}
	}
}

void MainWindow::stop(bool autoConfirmRecord, bool addEvents, bool resetPause){

	if(!file){
		return;
	}

	disconnect(MidiPlayer::playerThread(),
			SIGNAL(playerStopped()), this,	SLOT(stop()));

	if(resetPause){
		file->setPauseTick(-1);
		mw_matrixWidget->update();
	}
	if(!MidiInput::recording() && MidiPlayer::isPlaying()){
		MidiPlayer::stop();
        _miscWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		_trackWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);
		mw_matrixWidget->timeMsChanged(MidiPlayer::timeMs(), true);
		_trackWidget->setEnabled(true);
		_remoteServer->stop();

		panic();
	}

	if(MidiInput::recording()){
		MidiPlayer::stop();
		panic();
        _miscWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);
		_trackWidget->setEnabled(true);
		_remoteServer->stop();
		QMultiMap<int, MidiEvent*> events = MidiInput::endInput();

		RecordDialog *dialog = new RecordDialog(file, events, this);
		dialog->setModal(true);
		if(!autoConfirmRecord){
			dialog->show();
		} else {
			if(addEvents){
				dialog->enter();
			}
		}
	}
}

void MainWindow::forward(){
	if(!file) return;

	QList<TimeSignatureEvent*> *eventlist = new QList<TimeSignatureEvent*>;
	int ticksleft;
	int oldTick = file->cursorTick();
	if(file->pauseTick() >= 0){
		oldTick = file->pauseTick();
	}
	if(MidiPlayer::isPlaying() && !MidiInput::recording()){
		oldTick = file->tick(MidiPlayer::timeMs());
		stop(true);
	}
	int measure = file->measure(oldTick, oldTick, &eventlist, &ticksleft);

	int newTick = oldTick - ticksleft + eventlist->last()->ticksPerMeasure();
	file->setPauseTick(-1);
	if(newTick <= file->endTick()){
		file->setCursorTick(newTick);
		mw_matrixWidget->timeMsChanged(file->msOfTick(newTick), true);
	}
	mw_matrixWidget->update();
}

void MainWindow::back(){
	if(!file) return;

	QList<TimeSignatureEvent*> *eventlist = new QList<TimeSignatureEvent*>;
	int ticksleft;
	int oldTick = file->cursorTick();
	if(file->pauseTick() >= 0){
		oldTick = file->pauseTick();
	}
	if(MidiPlayer::isPlaying() && !MidiInput::recording()){
		oldTick = file->tick(MidiPlayer::timeMs());
		stop(true);
	}
	int measure = file->measure(oldTick, oldTick, &eventlist, &ticksleft);
	int newTick = oldTick;
	if(ticksleft > 0){
		newTick -= ticksleft;
	} else {
		newTick -= eventlist->last()->ticksPerMeasure();
	}
	measure = file->measure(newTick, newTick, &eventlist, &ticksleft);
	if(ticksleft > 0){
		newTick -= ticksleft;
	}
	file->setPauseTick(-1);
	if(newTick >= 0){
		file->setCursorTick(newTick);
		mw_matrixWidget->timeMsChanged(file->msOfTick(newTick), true);
	}
	mw_matrixWidget->update();
}

void MainWindow::backToBegin(){
	if(!file) return;

	file->setPauseTick(0);
	file->setCursorTick(0);

	mw_matrixWidget->update();
}

void MainWindow::save(){

	if(!file) return;

	if(QFile(file->path()).exists()){

		bool printMuteWarning = false;

		for(int i = 0; i<16; i++){
			MidiChannel *ch = file->channel(i);
			if(ch->mute() || !ch->visible()){
				printMuteWarning = true;
			}
		}
		foreach(MidiTrack *track , *(file->tracks())){
			if(track->muted() || track->hidden()){
				printMuteWarning = true;
			}
		}

		if(printMuteWarning){
			QMessageBox::information(this, "Channels/Tracks mute or invisible",
					"One or more channels/tracks are mute or invisible. They will be audible in the saved file!",
					"Save file", 0, 0);
		}

		if(!file->save(file->path())){
			QMessageBox::warning(this, "Error", QString("The file could not be saved!"));
		} else {
			setWindowModified(false);
		}
	} else {
		saveas();
	}
}

void MainWindow::saveas(){

	if(!file) return;

	QString oldPath = file->path();
	QFile *f = new QFile(oldPath);
	QString dir = startDirectory;
	if(f->exists()){
		QFileInfo(*f).dir().path();
	}
	QString newPath = QFileDialog::getSaveFileName(this, "Save file as...",
			dir);

	if(newPath == ""){
		return;
	}

	// automatically add '.mid' extension
	if(!newPath.endsWith(".mid", Qt::CaseInsensitive) && !newPath.endsWith(".midi", Qt::CaseInsensitive))
	{
		newPath.append(".mid");
	}

	if(file->save(newPath)){

		bool printMuteWarning = false;

		for(int i = 0; i<16; i++){
			MidiChannel *ch = file->channel(i);
			if(ch->mute() || !ch->visible()){
				printMuteWarning = true;
			}
		}
		foreach(MidiTrack *track , *(file->tracks())){
			if(track->muted() || track->hidden()){
				printMuteWarning = true;
			}
		}

		if(printMuteWarning){
			QMessageBox::information(this, "Channels/Tracks mute or invisible",
					"One or more channels/tracks are mute or invisible. They will be audible in the saved file!",
					"Save file", 0, 0);
		}

		file->setPath(newPath);
		setWindowTitle("MidiEditor - " +file->path()+"[*]");
		updateRecentPathsList();
		setWindowModified(false);
	} else {
		QMessageBox::warning(this, "Error", QString("The file could not be saved!"));
	}
}

void MainWindow::load(){
	QString oldPath = startDirectory;
	if(file){
		oldPath = file->path();
		if(!file->saved()){
			switch(QMessageBox::question(this, "Save file?", "save file "+
					file->path()+
				" before closing?", "Save","Close without saving", "Cancel",0,2))
			{
				case 0: {
					// save
					if(QFile(file->path()).exists()){
						file->save(file->path());
					} else {
						saveas();
					}
					break;
				}
				case 1: {
					// close
					break;
				}
				case 2: {
					// break
					return;
				}
			}
		}
	}

	QFile *f = new QFile(oldPath);
	QString dir = startDirectory;
	if(f->exists()){
		QFileInfo(*f).dir().path();
	}
	QString newPath = QFileDialog::getOpenFileName(this, "Open file",
			dir, "MIDI Files(*.mid *.midi);;All Files(*)");

	if(!newPath.isEmpty()){
		openFile(newPath);
	}
}

void MainWindow::openFile(QString filePath){

	bool ok = true;

	QFile nf(filePath);

	if(!nf.exists()){

		QMessageBox::warning(this, "Error", QString("The file does not exist!"));
		return;
	}

	startDirectory = QFileInfo(nf).absoluteDir().path()+"/";

	MidiFile *mf = new MidiFile(filePath, &ok);

	if(ok){
		setFile(mf);
		updateRecentPathsList();
	} else {
		QMessageBox::warning(this, "Error", QString("No file opened"));
	}
}

void MainWindow::redo(){
	if(file) file->protocol()->redo(true);
}

void MainWindow::undo(){
	if(file) file->protocol()->undo(true);
}

EventWidget *MainWindow::eventWidget(){
	return _eventWidget;
}

void MainWindow::muteAllChannels(){
	if(!file) return;
	file->protocol()->startNewAction("Mute all Channels");
	for(int i=0; i<19; i++){
		file->channel(i)->setMute(true);
	}
	file->protocol()->endAction();
	channelWidget->update();
}

void MainWindow::unmuteAllChannels(){
	if(!file) return;
	file->protocol()->startNewAction("Unmute all Channels");
	for(int i=0; i<19; i++){
		file->channel(i)->setMute(false);
	}
	file->protocol()->endAction();
	channelWidget->update();
}

void MainWindow::allChannelsVisible(){
	if(!file) return;
	file->protocol()->startNewAction("Set all Channels visible");
	for(int i=0; i<19; i++){
		file->channel(i)->setVisible(true);
	}
	file->protocol()->endAction();
	channelWidget->update();
}

void MainWindow::allChannelsInvisible(){
	if(!file) return;
	file->protocol()->startNewAction("Set all Channels invisible");
	for(int i=0; i<19; i++){
		file->channel(i)->setVisible(false);
	}
	file->protocol()->endAction();
	channelWidget->update();
}

void MainWindow::closeEvent(QCloseEvent *event){

	if(!file || file->saved()){
		event->accept();
	} else {
		switch(QMessageBox::question(this, "Save file?", "save file "+
				file->path()+
			" before closing?", "Save","Close without saving", "Cancel", 0,2))
		{
			case 0: {
				// save
				if(QFile(file->path()).exists()){
					file->save(file->path());
					event->accept();
				} else {
					saveas();
					event->accept();
				}
				break;
			}
			case 1: {
				// close
				event->accept();
				break;
			}
			case 2: {
				// break
				event->ignore();
				return;
			}
		}
	}

	if(MidiOutput::outputPort() != ""){
		_settings->setValue("out_port", MidiOutput::outputPort());
	}
	if(MidiInput::inputPort() != ""){
		_settings->setValue("in_port", MidiInput::inputPort());
	}
	if(_remoteServer->clientIp() != ""){
		_settings->setValue("udp_client_ip", _remoteServer->clientIp());
	}
	if(_remoteServer->clientPort() > 0){
		_settings->setValue("udp_client_port", _remoteServer->clientPort());
	}
	_remoteServer->stopServer();

	bool ok;
	int numStart = _settings->value("numStart", -1).toInt(&ok);
	_settings->setValue("numStart", numStart+1);

	// save the current Path
	_settings->setValue("open_path", startDirectory);
}

void MainWindow::donate(){
	DonateDialog *d = new DonateDialog(this);
	d->setModal(true);
	d->show();
}

void MainWindow::about(){
	AboutDialog *d = new AboutDialog(this);
	d->setModal(true);
	d->show();
}

void MainWindow::setFileLengthMs(){
	if(!file) return;

	FileLengthDialog *d = new FileLengthDialog(file, this);
	d->setModal(true);
	d->show();
}

void MainWindow::setStartDir(QString dir){
	startDirectory = dir;
}

void MainWindow::setStartupCmd(){
    bool ok;
    QString text = QInputDialog::getText(this, "Set Start Command",
                                         "Start Command", QLineEdit::Normal,
                                         _settings->value("start_cmd", "").
                                         toString(), &ok);
    if (ok && !text.isEmpty()){
    	_settings->setValue("start_cmd", text);
    }
}

void MainWindow::midiSettings(){
	MidiSettingsDialog *d = new MidiSettingsDialog(this);
	d->setModal(true);
	d->show();
}

void MainWindow::newFile(){
	if(file){
		if(!file->saved()){
			switch(QMessageBox::question(this, "Save file?", "save file "+
					file->path()+
				" before closing?", "Save","Close without saving", "Cancel",0,2))
			{
				case 0: {
					// save
					if(QFile(file->path()).exists()){
						file->save(file->path());
					} else {
						saveas();
					}
					break;
				}
				case 1: {
					// close
					break;
				}
				case 2: {
					// break
					return;
				}
			}
		}
	}

	// create new File
	MidiFile *f = new MidiFile();
	setFile(f);
	setWindowTitle("MidiEditor - Untitled Document[*]");

	bool ok;
	int numStart = _settings->value("numStart", -1).toInt(&ok);
	if(numStart == 15){
		donate();
	}
}

void MainWindow::panic(){
	MidiPlayer::panic();
}

void MainWindow::toggleScreenLock() {
	if(mw_matrixWidget->screenLocked()){
		_lockAction->setIcon(QIcon("graphics/tool/screen_unlocked.png"));
		_lockAction->setToolTip("Do not scroll automatically while playing/recording");
		mw_matrixWidget->setScreenLocked(false);
	} else {
		_lockAction->setIcon(QIcon("graphics/tool/screen_locked.png"));
		_lockAction->setToolTip("Scroll automatically while playing/recording");
		mw_matrixWidget->setScreenLocked(true);
	}
}

void MainWindow::scaleSelection(){
    bool ok;
    double scale = QInputDialog::getDouble(this, "Scalefactor",
    		"Scalefactor:", 1.0, 0, 2147483647, 1, &ok);
    if (ok && scale>0 && EventTool::selectedEventList()->size()>0 && file){
    	// find minimum
    	int minTime = 2147483647;
    	foreach(MidiEvent *e, *EventTool::selectedEventList()){
    		if(e->midiTime() < minTime){
    			minTime = e->midiTime();
    		}
    	}

    	file->protocol()->startNewAction("Scale events", 0);
    	foreach(MidiEvent *e, *EventTool::selectedEventList()){
    		e->setMidiTime((e->midiTime()-minTime)*scale + minTime);
    		OnEvent *on = dynamic_cast<OnEvent*>(e);
    		if(on){
    			MidiEvent *off = on->offEvent();
    			off->setMidiTime((off->midiTime()-minTime)*scale + minTime);
    		}
    	}
    	file->protocol()->endAction();
    }
}

void MainWindow::alignLeft(){
    if (EventTool::selectedEventList()->size()>1 && file){
        // find minimum
        int minTime = 2147483647;
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                if(e->midiTime() < minTime){
                        minTime = e->midiTime();
                }
        }

        file->protocol()->startNewAction("Align Left events", 0);
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                int onTime = e->midiTime();
                e->setMidiTime(minTime);
                OnEvent *on = dynamic_cast<OnEvent*>(e);
                if(on){
                        MidiEvent *off = on->offEvent();
                        off->setMidiTime(minTime + (off->midiTime()-onTime));
                }
        }
        file->protocol()->endAction();
    }
}

void MainWindow::alignRight(){
    if (EventTool::selectedEventList()->size()>1 && file){
        // find maximum
        int maxTime = 0;
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                OnEvent *on = dynamic_cast<OnEvent*>(e);
                MidiEvent *off = on->offEvent();
                if(off->midiTime() > maxTime){
                        maxTime = off->midiTime();
                }
        }

        file->protocol()->startNewAction("Align Right events", 0);
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                int onTime = e->midiTime();
                OnEvent *on = dynamic_cast<OnEvent*>(e);
                if(on){
                        MidiEvent *off = on->offEvent();
                        e->setMidiTime(maxTime - (off->midiTime()-onTime));
                        off->setMidiTime(maxTime);
                }
        }
        file->protocol()->endAction();
    }
}

void MainWindow::equalize()
{
    if (EventTool::selectedEventList()->size()>1 && file){
        // find average
        int avgStart = 0;
        int avgTime = 0;
        int count = 0;
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                OnEvent *on = dynamic_cast<OnEvent*>(e);
                MidiEvent *off = on->offEvent();
                avgStart += e->midiTime();
                avgTime += (off->midiTime() - e->midiTime());
                count++;
        }
        avgStart /= count;
        avgTime /= count;

        file->protocol()->startNewAction("Equalize events", 0);
        foreach(MidiEvent *e, *EventTool::selectedEventList()){
                OnEvent *on = dynamic_cast<OnEvent*>(e);
                if(on){
                        MidiEvent *off = on->offEvent();
                        e->setMidiTime(avgStart);
                        off->setMidiTime(avgStart + avgTime);
                }
        }
        file->protocol()->endAction();
    }
}

void MainWindow::deleteSelectedEvents(){
	bool showsSelected = false;
	if(Tool::currentTool()){
		EventTool *eventTool = dynamic_cast<EventTool*>(Tool::currentTool());
		if(eventTool){
			showsSelected = eventTool->showsSelection();
		}
	}
	if(showsSelected && EventTool::selectedEventList()->size()>0 && file){

		file->protocol()->startNewAction("Remove Event(s)");
		foreach(MidiEvent *ev, *EventTool::selectedEventList()){
			file->channel(ev->channel())->removeEvent(ev);
		}
		EventTool::selectedEventList()->clear();
    	file->protocol()->endAction();
    }
}

void MainWindow::deleteChannel(QAction *action){

	if(!file){
		return;
	}

	int num = action->data().toInt();
	file->protocol()->startNewAction("Remove all events from channel "+QString::number(num));
	file->channel(num)->deleteAllEvents();
	file->protocol()->endAction();
}


void MainWindow::moveSelectedEventsToChannel(QAction *action){

	if(!file){
		return;
	}

	int num = action->data().toInt();
	MidiChannel *channel = file->channel(num);

    if (EventTool::selectedEventList()->size()>0){
    	file->protocol()->startNewAction("Move selected events to channel "+QString::number(num));
		foreach(MidiEvent *ev, *EventTool::selectedEventList()){
			file->channel(ev->channel())->removeEvent(ev);
			ev->setChannel(num, true);
			OnEvent *onevent = dynamic_cast<OnEvent*>(ev);
			if(onevent){
				channel->insertEvent(onevent->offEvent(), onevent->offEvent()->midiTime());
				onevent->offEvent()->setChannel(num);
			}
			channel->insertEvent(ev, ev->midiTime());
		}

    	file->protocol()->endAction();
    }
}

void MainWindow::moveSelectedEventsToTrack(QAction *action){

	if(!file){
		return;
	}

	int num = action->data().toInt();
	MidiChannel *channel = file->channel(num);

    if (EventTool::selectedEventList()->size()>0){
    	file->protocol()->startNewAction("Move selected events to track "+QString::number(num));
		foreach(MidiEvent *ev, *EventTool::selectedEventList()){
			ev->setTrack(num, true);
			OnEvent *onevent = dynamic_cast<OnEvent*>(ev);
			if(onevent){
				onevent->offEvent()->setTrack(num);
			}
			channel->insertEvent(ev, ev->midiTime());
		}

    	file->protocol()->endAction();
    }
}

void MainWindow::updateRecentPathsList(){

	// if file opened put it at the top of the list
	if(file){

		QString currentPath = file->path();
		QStringList newList;
		newList.append(currentPath);

		foreach(QString str, _recentFilePaths){
			if(str != currentPath && newList.size()<10){
				newList.append(str);
			}
		}

		_recentFilePaths = newList;
	}

	// save list
	QVariant list(_recentFilePaths);
	_settings->setValue("recent_file_list", list);

	// update menu
	_recentPathsMenu->clear();
	foreach(QString path, _recentFilePaths){
		QFile f(path);
		QString name = QFileInfo(f).fileName();

		QVariant variant(path);
		QAction *openRecentFileAction = new QAction(name, this);
		openRecentFileAction->setData(variant);
		_recentPathsMenu->addAction(openRecentFileAction);
	}

}

void MainWindow::openRecent(QAction *action){

	QString  path = action->data().toString();

	if(file){
		QString oldPath = file->path();

		if(!file->saved()){
			switch(QMessageBox::question(this, "Save file?", "save file "+
					file->path()+
				" before closing?", "Save","Close without saving", "Cancel",0,2))
			{
				case 0: {
					// save
					if(QFile(file->path()).exists()){
						file->save(file->path());
					} else {
						saveas();
					}
					break;
				}
				case 1: {
					// close
					break;
				}
				case 2: {
					// break
					return;
				}
			}
		}
	}

	openFile(path);
}


void MainWindow::updateChannelMenu() {


	// visibilitymenu
	foreach(QAction *action, _channelVisibilityMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->visible());
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		} else {
			action->setChecked(false);
		}
	}

	// mute menu
	foreach(QAction *action, _channelMuteMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->mute());
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		} else {
			action->setChecked(false);
		}
	}

	// solo menu
	foreach(QAction *action, _channelSoloMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->solo());
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		} else {
			action->setChecked(false);
		}
	}

	// delete channel events menu
	foreach(QAction *action, _deleteChannelMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		}
	}

	// channel Instrument Menu
	foreach(QAction *action, _channelInstrumentMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		}
	}

	// move events to channel...
	foreach(QAction *action, _moveSelectedEventsToChannelMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setText(QString::number(channel)+" "+MidiFile::instrumentName(file->channel(channel)->progAtTick(0)));
		}
	}

	_chooseEditChannel->setCurrentIndex(NewNoteTool::editChannel());
}

void MainWindow::updateTrackMenu() {

	_renameTrackMenu->clear();
	_removeTrackMenu->clear();
	_moveSelectedEventsToTrackMenu->clear();
	_trackMuteMenu->clear();
	_trackVisibilityMenu->clear();
	_chooseEditTrack->clear();

	if(!file){
		return;
	}

	for(int i = 0; i<file->numTracks(); i++){
		QVariant variant(i);
		QAction *renameTrackAction = new QAction(QString::number(i)+" "+file->tracks()->at(i)->name(), this);
		renameTrackAction->setData(variant);
		_renameTrackMenu->addAction(renameTrackAction);
	}

	for(int i = 0; i<file->numTracks(); i++){
		QVariant variant(i);
		QAction *removeTrackAction = new QAction(QString::number(i)+" "+file->tracks()->at(i)->name(), this);
		removeTrackAction->setData(variant);
		_removeTrackMenu->addAction(removeTrackAction);
	}

	for(int i = 0; i<file->numTracks(); i++){
		QVariant variant(i);
		QAction *moveToTrackAction = new QAction(QString::number(i)+" "+file->tracks()->at(i)->name(), this);
		moveToTrackAction->setData(variant);
		_moveSelectedEventsToTrackMenu->addAction(moveToTrackAction);
	}

	for(int i = 0; i<file->numTracks(); i++){
		QVariant variant(i);
		QAction *muteTrackAction = new QAction(QString::number(i)+" "+file->tracks()->at(i)->name(), this);
		muteTrackAction->setData(variant);
		_trackMuteMenu->addAction(muteTrackAction);
		muteTrackAction->setCheckable(true);
		muteTrackAction->setChecked(file->track(i)->muted());
	}

	for(int i = 0; i<file->numTracks(); i++){
		QVariant variant(i);
		QAction *showTrackAction = new QAction(QString::number(i)+" "+file->tracks()->at(i)->name(), this);
		showTrackAction->setData(variant);
		_trackVisibilityMenu->addAction(showTrackAction);
		showTrackAction->setCheckable(true);
		showTrackAction->setChecked(!(file->track(i)->hidden()));
	}

	for(int i = 0; i<file->numTracks(); i++){
		_chooseEditTrack->addItem("Track "+QString::number(i)+": "+file->tracks()->at(i)->name());
	}
	if(NewNoteTool::editTrack() >= file->numTracks()){
		NewNoteTool::setEditTrack(0);
	}
	_chooseEditTrack->setCurrentIndex(NewNoteTool::editTrack());
}

void MainWindow::muteChannel(QAction *action){
	int channel = action->data().toInt();
	if(file){
		file->protocol()->startNewAction("Mute channel");
		file->channel(channel)->setMute(action->isChecked());
		updateChannelMenu();
		channelWidget->update();
		file->protocol()->endAction();
	}
}
void MainWindow::soloChannel(QAction *action){
	int channel = action->data().toInt();
	if(file){
		file->protocol()->startNewAction("Select solo channel");
		for(int i = 0; i<16; i++){
			file->channel(i)->setSolo(i==channel && action->isChecked());
		}
		file->protocol()->endAction();
	}
	channelWidget->update();
	updateChannelMenu();
}

void MainWindow::viewChannel(QAction *action){
	int channel = action->data().toInt();
	if(file){
		file->protocol()->startNewAction("Channel visibility changed");
		file->channel(channel)->setVisible(action->isChecked());
		updateChannelMenu();
		channelWidget->update();
		file->protocol()->endAction();
	}
}

void MainWindow::keyPressEvent(QKeyEvent *event){
	mw_matrixWidget->takeKeyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
	mw_matrixWidget->takeKeyReleaseEvent(event);
}

void MainWindow::showEventWidget(MidiEvent *event){
	if(event){
		lowerTabWidget->setCurrentIndex(1);
	} else {
		lowerTabWidget->setCurrentIndex(0);
	}
}

void MainWindow::renameTrackMenuClicked(QAction *action){
	int track = action->data().toInt();
	renameTrack(track);
}

void MainWindow::renameTrack(int tracknumber){

	if(!file){
		return;
	}

	file->protocol()->startNewAction("Edit Track Name");

    bool ok;
    QString text = QInputDialog::getText(this, "Set Track Name",
         "Track Name (Track "+QString::number(tracknumber)+")", QLineEdit::Normal,
         file->tracks()->at(tracknumber)->name(), &ok);
    if (ok && !text.isEmpty()){
    	file->tracks()->at(tracknumber)->setName(text);
    }

	file->protocol()->endAction();
	updateTrackMenu();
}

void MainWindow::removeTrackMenuClicked(QAction *action){
	int track = action->data().toInt();
	removeTrack(track);
}

void MainWindow::removeTrack(int tracknumber){

	if(!file){
		return;
	}

	file->protocol()->startNewAction("Remove Track");
	if(!file->removeTrack(tracknumber)){
		QMessageBox::warning(this, "Error", QString("The selected Track can\'t be removed!\n It\'s the last Track of the File!"));
	}
	file->protocol()->endAction();
	updateTrackMenu();
}

void MainWindow::addTrack(){

	if(file){

		bool ok;
		QString text = QInputDialog::getText(this, "Set Track Name",
			 "Track Name (New Track)", QLineEdit::Normal,
			 "New Track", &ok);
		if (ok && !text.isEmpty()){


			file->protocol()->startNewAction("Add Track");
			file->addTrack();
			file->tracks()->at(file->numTracks()-1)->setName(text);
			file->protocol()->endAction();

			updateTrackMenu();
		}
	}
}

void MainWindow::muteAllTracks(){
	if(!file) return;
	file->protocol()->startNewAction("Mute all Tracks");
	foreach(MidiTrack *track, *(file->tracks())){
		track->setMuted(true);
	}
	file->protocol()->endAction();
	_trackWidget->update();
}

void MainWindow::unmuteAllTracks(){
	if(!file) return;
	file->protocol()->startNewAction("Unmute all Tracks");
	foreach(MidiTrack *track, *(file->tracks())){
		track->setMuted(false);
	}
	file->protocol()->endAction();
	_trackWidget->update();
}

void MainWindow::allTracksVisible(){
	if(!file) return;
	file->protocol()->startNewAction("Show all Tracks");
	foreach(MidiTrack *track, *(file->tracks())){
		track->setHidden(false);
	}
	file->protocol()->endAction();
	_trackWidget->update();
}

void MainWindow::allTracksInvisible(){
	if(!file) return;
	file->protocol()->startNewAction("Hide all Tracks");
	foreach(MidiTrack *track, *(file->tracks())){
		track->setHidden(true);
	}
	file->protocol()->endAction();
	_trackWidget->update();
}

void MainWindow::showTrackMenuClicked(QAction *action){
	int track = action->data().toInt();
	if(file){
		file->protocol()->startNewAction("Show Track");
		file->track(track)->setHidden(!(action->isChecked()));
		updateTrackMenu();
		_trackWidget->update();
		file->protocol()->endAction();
	}
}

void MainWindow::muteTrackMenuClicked(QAction *action){
	int track = action->data().toInt();
	if(file){
		file->protocol()->startNewAction("Mute Track");
		file->track(track)->setMuted(action->isChecked());
		updateTrackMenu();
		_trackWidget->update();
		file->protocol()->endAction();
	}
}

void MainWindow::selectAll(){

	if(!file){
		return;
	}

	file->protocol()->startNewAction("Select all");

	for(int i = 0; i<16; i++){
		foreach(MidiEvent *event, file->channel(i)->eventMap()->values()){
			EventTool::selectEvent(event, false, true);
		}
	}

	file->protocol()->endAction();
}

void MainWindow::transposeNSemitones(){

	if(!file){
		return;
	}

	QList<NoteOnEvent*> events;
	foreach(MidiEvent *event, *EventTool::selectedEventList()){
		NoteOnEvent *on = dynamic_cast<NoteOnEvent*>(event);
		if(on){
			events.append(on);
		}
	}

	TransposeDialog *d = new TransposeDialog(events, file, this);
	d->setModal(true);
	d->show();
}

void MainWindow::copy(){
	EventTool::copyAction();
}
void MainWindow::paste(){
	EventTool::pasteAction();
}

void MainWindow::markEdited(){
	setWindowModified(true);
}

void MainWindow::colorsByChannel(){
	mw_matrixWidget->setColorsByChannel();
	_colorsByChannel->setChecked(true);
	_colorsByTracks->setChecked(false);
	mw_matrixWidget->registerRelayout();
	mw_matrixWidget->update();
    _miscWidget->update();
}
void MainWindow::colorsByTrack(){
	mw_matrixWidget->setColorsByTracks();
	_colorsByChannel->setChecked(false);
	_colorsByTracks->setChecked(true);
	mw_matrixWidget->registerRelayout();
	mw_matrixWidget->update();
    _miscWidget->update();
}

void MainWindow::editChannel(int i){
	NewNoteTool::setEditChannel(i);
	updateChannelMenu();
}
void MainWindow::editTrack(int i){
	NewNoteTool::setEditTrack(i);
	updateTrackMenu();
}

void MainWindow::setInstrumentForChannel(int i){
	InstrumentChooser *d = new InstrumentChooser(file, i, this);
	d->setModal(true);
	d->exec();

	updateChannelMenu();
}

void MainWindow::instrumentChannel(QAction *action){
	if(file){
		setInstrumentForChannel(action->data().toInt());
	}
}

void MainWindow::spreadSelection(){

	if(!file){
		return;
	}

	bool ok;
	float numMs = QInputDialog::getDouble(this, "Set spread-time",
		"Spread time [ms]", 10,
		5,500, 2, &ok);

	if(!ok){
		numMs = 1;
	}

	QMultiMap<int, int> spreadChannel[19];

	foreach(MidiEvent *event, *EventTool::selectedEventList()){
		if(!spreadChannel[event->channel()].values(event->line()).contains(event->midiTime())){
			spreadChannel[event->channel()].insert(event->line(), event->midiTime());
		}
	}

	file->protocol()->startNewAction("Spread Events");
	int numSpreads = 0;
	for(int i = 0; i<19;i++){

		MidiChannel *channel = file->channel(i);

		QList<int> seenBefore;

		foreach(int line, spreadChannel[i].keys()){

			if(seenBefore.contains(line)){
				continue;
			}

			seenBefore.append(line);

			foreach(int position, spreadChannel[i].values(line)){

				QList<MidiEvent*> eventsWithAllLines = channel->eventMap()->values(position);

				QList<MidiEvent*> events;
				foreach(MidiEvent *event, eventsWithAllLines){
					if(event->line() == line){
						events.append(event);
					}
				}

				//spread events for the channel at the given position
				int num = events.count();
				if(num>1){

					float timeToInsert = file->msOfTick(position)+numMs*num/2;


					for(int y = 0; y<num; y++){

						MidiEvent *toMove = events.at(y);

						toMove->setMidiTime(file->tick(timeToInsert), true);
						numSpreads++;

						timeToInsert -= numMs;
					}
				}
			}
		}
	}
	file->protocol()->endAction();

	QMessageBox::information(this, "Spreading done", QString("Spreaded "+QString::number(numSpreads)+" Events"));
}

void MainWindow::showRemoteDialog(){
	RemoteDialog *remoteDialog = new RemoteDialog(_remoteServer, this);
	remoteDialog->setModal(true);
	remoteDialog->show();
}

void MainWindow::manual(){

	QProcess *process = new QProcess;
	process->setWorkingDirectory("assistant");
	QStringList args;
	args << QLatin1String("-collectionFile")
	<< QLatin1String("midieditor-collection.qhc");

#ifdef __WINDOWS_MM__
	process->start(QLatin1String("assistant/assistant"), args);
#else
	process->start(QLatin1String("assistant"), args);
#endif

	if (!process->waitForStarted())
	return;

}

void MainWindow::changeMiscMode(int mode){
    _miscWidget->setMode(mode);
    if(mode == VelocityEditor){
        _miscChannel->setEnabled(false);
    } else {
        _miscChannel->setEnabled(true);
    }
	if(mode == ControllEditor || mode == KeyPressureEditor){
        _miscController->setEnabled(true);
		_miscController->clear();

		if(mode == ControllEditor){
			for(int i = 0; i<128; i++){
				_miscController->addItem(MidiFile::controlChangeName(i));
			}
		} else {
			for(int i = 0; i<128; i++){
				_miscController->addItem("Note: "+QString::number(i));
			}
		}
    } else {
        _miscController->setEnabled(false);
    }
}

void MainWindow::selectModeChanged(QAction *action){
    if(action == setSingleMode){
        _miscWidget->setEditMode(SINGLE_MODE);
    }
    if(action == setLineMode){
        _miscWidget->setEditMode(LINE_MODE);
    }
    if(action == setFreehandMode){
        _miscWidget->setEditMode(MOUSE_MODE);
    }
}

QWidget *MainWindow::setupActions(QWidget *parent){

    // Menubar
    QMenu *fileMB = menuBar()->addMenu("File");
    QMenu *editMB = menuBar()->addMenu("Edit");
    QMenu *channelsMB = menuBar()->addMenu("Channels");
    QMenu *tracksMB = menuBar()->addMenu("Tracks");
    QMenu *timingMB = menuBar()->addMenu("Timing");
    QMenu *viewMB = menuBar()->addMenu("View");
    QMenu *playbackMB = menuBar()->addMenu("Playback");
    QMenu *remoteMB = menuBar()->addMenu("Remote");
    QMenu *midiMB = menuBar()->addMenu("Midi");
    QMenu *helpMB = menuBar()->addMenu("Help");

    // File
    QAction *newAction = new QAction("New File", this);
	newAction->setShortcut(QKeySequence::New);
    newAction->setIcon(QIcon("graphics/tool/new.png"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
    fileMB->addAction(newAction);

    QAction *loadAction = new QAction("Open File...", this);
	loadAction->setShortcut(QKeySequence::Open);
    loadAction->setIcon(QIcon("graphics/tool/load.png"));
    connect(loadAction, SIGNAL(triggered()), this, SLOT(load()));
    fileMB->addAction(loadAction);

    _recentPathsMenu = new QMenu("Open Recent..", this);
	_recentPathsMenu->setIcon(QIcon("graphics/tool/noicon.png"));
    fileMB->addMenu(_recentPathsMenu);
    connect(_recentPathsMenu, SIGNAL(triggered(QAction*)), this, SLOT(openRecent(QAction*)));

    updateRecentPathsList();

    fileMB->addSeparator();

    QAction *saveAction = new QAction("Save File", this);
	saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon("graphics/tool/save.png"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    fileMB->addAction(saveAction);

    QAction *saveAsAction = new QAction("Save As...", this);
	saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon("graphics/tool/saveas.png"));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveas()));
    fileMB->addAction(saveAsAction);

    fileMB->addSeparator();

    QAction *quitAction = new QAction("Quit", this);
	quitAction->setShortcut(QKeySequence::Quit);
	quitAction->setIcon(QIcon("graphics/tool/noicon.png"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    fileMB->addAction(quitAction);

    // Edit
    QAction *undoAction = new QAction("Undo", this);
	undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon("graphics/tool/undo.png"));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    editMB->addAction(undoAction);

    QAction *redoAction = new QAction("Redo", this);
	redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon("graphics/tool/redo.png"));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    editMB->addAction(redoAction);

    editMB->addSeparator();

    QAction *selectAllAction = new QAction("Select all Visible Events", this);
	selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
    editMB->addAction(selectAllAction);


    editMB->addSeparator();

    QAction *copyAction = new QAction("Copy Events to Clipboard", this);
	copyAction->setIcon(QIcon("graphics/tool/copy.png"));
	copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    editMB->addAction(copyAction);

    QAction *pasteAction = new QAction("Paste Events, beginning at the Cursor Position", this);
	pasteAction->setShortcut(QKeySequence::Paste);
	pasteAction->setIcon(QIcon("graphics/tool/paste.png"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    editMB->addAction(pasteAction);

    editMB->addSeparator();

    QAction *deleteAction = new QAction("Remove selected Events", this);
	deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon("graphics/tool/eraser.png"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSelectedEvents()));
    editMB->addAction(deleteAction);

    editMB->addSeparator();

    QAction *alignLeftAction = new QAction("Align Events to Leftmost", this);
    alignLeftAction->setIcon(QIcon("graphics/tool/align_left.png"));
    connect(alignLeftAction, SIGNAL(triggered()), this, SLOT(alignLeft()));
    editMB->addAction(alignLeftAction);

    QAction *alignRightAction = new QAction("Align Events to Rightmost", this);
    alignRightAction->setIcon(QIcon("graphics/tool/align_right.png"));
    connect(alignRightAction, SIGNAL(triggered()), this, SLOT(alignRight()));
    editMB->addAction(alignRightAction);

    QAction *equalizeAction = new QAction("Equalize Selection", this);
    equalizeAction->setIcon(QIcon("graphics/tool/equalize.png"));
    connect(equalizeAction, SIGNAL(triggered()), this, SLOT(equalize()));
    editMB->addAction(equalizeAction);

    editMB->addSeparator();

    QAction *spreadAction = new QAction("Spread Selection", this);
    connect(spreadAction, SIGNAL(triggered()), this, SLOT(spreadSelection()));
    editMB->addAction(spreadAction);

    editMB->addSeparator();

    _moveSelectedEventsToChannelMenu = new QMenu("Move selected Events to Channel...", editMB);
    editMB->addMenu(_moveSelectedEventsToChannelMenu);
    connect(_moveSelectedEventsToChannelMenu, SIGNAL(triggered(QAction*)), this, SLOT(moveSelectedEventsToChannel(QAction*)));

    for(int i = 0; i<16; i++){
        QVariant variant(i);
        QAction *moveToChannelAction = new QAction(QString::number(i), this);
        moveToChannelAction->setData(variant);
        _moveSelectedEventsToChannelMenu->addAction(moveToChannelAction);
    }

    _moveSelectedEventsToTrackMenu = new QMenu("Move selected Events to Track...", editMB);
    editMB->addMenu(_moveSelectedEventsToTrackMenu);
    connect(_moveSelectedEventsToTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(moveSelectedEventsToTrack(QAction*)));

    editMB->addSeparator();

    QAction *transposeAction = new QAction("Transpose Selection...", this);
    connect(transposeAction, SIGNAL(triggered()), this, SLOT(transposeNSemitones()));
    editMB->addAction(transposeAction);

    // channels
    QAction *allChannelsVisible = new QAction("Show all Channels", this);
    allChannelsVisible->setIcon(QIcon("graphics/channelwidget/visible.png"));
    connect(allChannelsVisible, SIGNAL(triggered()), this,
            SLOT(allChannelsVisible()));
    channelsMB->addAction(allChannelsVisible);

    QAction *allChannelsInvisible = new QAction("Hide all Channels", this);
    allChannelsInvisible->setIcon(QIcon("graphics/channelwidget/hidden.png"));
    connect(allChannelsInvisible, SIGNAL(triggered()), this,
            SLOT(allChannelsInvisible()));
    channelsMB->addAction(allChannelsInvisible);

    _channelVisibilityMenu = new QMenu("Shown Channels...", channelsMB);
    channelsMB->addMenu(_channelVisibilityMenu);
    connect(_channelVisibilityMenu, SIGNAL(triggered(QAction*)), this, SLOT(viewChannel(QAction*)));

    for(int i = 0; i<17; i++){
        QVariant variant(i);
        QAction *viewChannelAction = new QAction(QString::number(i), this);
        viewChannelAction->setCheckable(true);
        viewChannelAction->setData(variant);
        _channelVisibilityMenu->addAction(viewChannelAction);
    }

    channelsMB->addSeparator();

    QAction *muteAllChannels = new QAction("Mute all Channels", this);
    muteAllChannels->setIcon(QIcon("graphics/channelwidget/mute.png"));
    connect(muteAllChannels, SIGNAL(triggered()), this,
            SLOT(muteAllChannels()));
    channelsMB->addAction(muteAllChannels);

    QAction *unmuteAllChannels = new QAction("Unmute all Channels", this);
    unmuteAllChannels->setIcon(QIcon("graphics/channelwidget/loud.png"));
    connect(unmuteAllChannels, SIGNAL(triggered()), this,
            SLOT(unmuteAllChannels()));
    channelsMB->addAction(unmuteAllChannels);

    _channelMuteMenu = new QMenu("Muted Channels...", channelsMB);
    channelsMB->addMenu(_channelMuteMenu);
    connect(_channelMuteMenu, SIGNAL(triggered(QAction*)), this, SLOT(muteChannel(QAction*)));

    for(int i = 0; i<16; i++){
        QVariant variant(i);
        QAction *muteChannelAction = new QAction(QString::number(i), this);
        muteChannelAction->setCheckable(true);
        muteChannelAction->setData(variant);
        _channelMuteMenu->addAction(muteChannelAction);
    }

    _channelSoloMenu = new QMenu("Select solo Channel...", channelsMB);
    channelsMB->addMenu(_channelSoloMenu);
    connect(_channelSoloMenu, SIGNAL(triggered(QAction*)), this, SLOT(soloChannel(QAction*)));

    for(int i = 0; i<16; i++){
        QVariant variant(i);
        QAction *soloChannelAction = new QAction(QString::number(i), this);
        soloChannelAction->setCheckable(true);
        soloChannelAction->setData(variant);
        _channelSoloMenu->addAction(soloChannelAction);
    }

    channelsMB->addSeparator();


    _deleteChannelMenu = new QMenu("Remove all Events from Channel...", channelsMB);
    channelsMB->addMenu(_deleteChannelMenu);
    connect(_deleteChannelMenu, SIGNAL(triggered(QAction*)), this, SLOT( deleteChannel(QAction*)));

    for(int i = 0; i<16; i++){
        QVariant variant(i);
        QAction *delChannelAction = new QAction(QString::number(i), this);
        delChannelAction->setData(variant);
        _deleteChannelMenu->addAction(delChannelAction);
    }

    _channelInstrumentMenu = new QMenu("Select Intrument for Channel...");
    channelsMB->addMenu(_channelInstrumentMenu);
    connect(_channelInstrumentMenu, SIGNAL(triggered(QAction*)), this, SLOT(instrumentChannel(QAction*)));

    for(int i = 0; i<16; i++){
        QVariant variant(i);
        QAction *instrumentChannelAction = new QAction(QString::number(i), this);
        instrumentChannelAction->setData(variant);
        _channelInstrumentMenu->addAction(instrumentChannelAction);
    }

    // Tracks
    QAction *addTrackAction = new QAction("Add Track...", tracksMB);
    tracksMB->addAction(addTrackAction);
    connect(addTrackAction, SIGNAL(triggered()), this, SLOT(addTrack()));

    tracksMB->addSeparator();

    QAction *allTracksVisible = new QAction("Show all Tracks", this);
    allTracksVisible->setIcon(QIcon("graphics/trackwidget/visible.png"));
    connect(allTracksVisible, SIGNAL(triggered()), this,
            SLOT(allTracksVisible()));
    tracksMB->addAction(allTracksVisible);

    QAction *allTracksInvisible = new QAction("Hide all Tracks", this);
    allTracksInvisible->setIcon(QIcon("graphics/trackwidget/hidden.png"));
    connect(allTracksInvisible, SIGNAL(triggered()), this,
            SLOT(allTracksInvisible()));
    tracksMB->addAction(allTracksInvisible);

    _trackVisibilityMenu = new QMenu("Shown Tracks...", tracksMB);
    tracksMB->addMenu(_trackVisibilityMenu);
    connect(_trackVisibilityMenu, SIGNAL(triggered(QAction*)), this, SLOT(showTrackMenuClicked(QAction*)));

    tracksMB->addSeparator();

    QAction *muteAllTracks = new QAction("Mute all Tracks", this);
    muteAllTracks->setIcon(QIcon("graphics/trackwidget/mute.png"));
    connect(muteAllTracks, SIGNAL(triggered()), this,
            SLOT(muteAllTracks()));
    tracksMB->addAction(muteAllTracks);

    QAction *unmuteAllTracks = new QAction("Unmute all Tracks", this);
    unmuteAllTracks->setIcon(QIcon("graphics/trackwidget/loud.png"));
    connect(unmuteAllTracks, SIGNAL(triggered()), this,
            SLOT(unmuteAllTracks()));
    tracksMB->addAction(unmuteAllTracks);

    _trackMuteMenu = new QMenu("Muted Tracks...", tracksMB);
    tracksMB->addMenu(_trackMuteMenu);
    connect(_trackMuteMenu, SIGNAL(triggered(QAction*)), this, SLOT(muteTrackMenuClicked(QAction*)));

    tracksMB->addSeparator();

    _removeTrackMenu = new QMenu("Remove Track...", tracksMB);
    tracksMB->addMenu(_removeTrackMenu);
    connect(_removeTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(removeTrackMenuClicked(QAction*)));

    _renameTrackMenu = new QMenu("Rename Track...", tracksMB);
    tracksMB->addMenu(_renameTrackMenu);
    connect(_renameTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(renameTrackMenuClicked(QAction*)));

    // Timing
    QAction *setFileLengthMs = new QAction("Set File Length (ms)", this);
    connect(setFileLengthMs, SIGNAL(triggered()), this,SLOT(setFileLengthMs()));
    timingMB->addAction(setFileLengthMs);

    QAction *scaleSelection = new QAction("Scale selected Events", this);
    connect(scaleSelection, SIGNAL(triggered()), this,SLOT(scaleSelection()));
    timingMB->addAction(scaleSelection);

    // View
    QMenu *_zoomMenu = new QMenu("Zoom...", tracksMB);
    QAction *zoomHorOutAction = new QAction("Out(horizontal)", this);
    zoomHorOutAction->setIcon(QIcon("graphics/tool/zoom_hor_out.png"));
    connect(zoomHorOutAction, SIGNAL(triggered()),
            mw_matrixWidget,SLOT(zoomHorOut()));
    _zoomMenu->addAction(zoomHorOutAction);

    QAction *zoomHorInAction = new QAction("In(horizontal)", this);
    zoomHorInAction->setIcon(QIcon("graphics/tool/zoom_hor_in.png"));
    connect(zoomHorInAction, SIGNAL(triggered()),
            mw_matrixWidget,SLOT(zoomHorIn()));
    _zoomMenu->addAction(zoomHorInAction);

    QAction *zoomVerOutAction = new QAction("Out(vertical)", this);
    zoomVerOutAction->setIcon(QIcon("graphics/tool/zoom_ver_out.png"));
    connect(zoomVerOutAction, SIGNAL(triggered()),
            mw_matrixWidget,SLOT(zoomVerOut()));
    _zoomMenu->addAction(zoomVerOutAction);

    QAction *zoomVerInAction = new QAction("In(vertical)", this);
    zoomVerInAction->setIcon(QIcon("graphics/tool/zoom_ver_in.png"));
    connect(zoomVerInAction, SIGNAL(triggered()),
            mw_matrixWidget,SLOT(zoomVerIn()));
    _zoomMenu->addAction(zoomVerInAction);

    viewMB->addMenu(_zoomMenu);

    viewMB->addSeparator();

    QMenu *colorMenu = new QMenu("Set Event Colors...", tracksMB);
    _colorsByChannel = new QAction("By Channels", this);
    _colorsByChannel->setCheckable(true);
    connect(_colorsByChannel, SIGNAL(triggered()), this,SLOT(colorsByChannel()));
    colorMenu->addAction(_colorsByChannel);

    _colorsByTracks = new QAction("By Tracks", this);
    _colorsByTracks->setCheckable(true);
    connect(_colorsByTracks, SIGNAL(triggered()), this,SLOT(colorsByTrack()));
    colorMenu->addAction(_colorsByTracks);

    viewMB->addMenu(colorMenu);
    colorsByChannel();

    // Playback
    QAction *playAction = new QAction("Play", this);
    playAction->setIcon(QIcon("graphics/tool/play.png"));
    connect(playAction, SIGNAL(triggered()), this, SLOT(play()));
    playbackMB->addAction(playAction);

    QAction *pauseAction = new QAction("Pause", this);
    pauseAction->setIcon(QIcon("graphics/tool/pause.png"));
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));
    playbackMB->addAction(pauseAction);

    QAction *recAction = new QAction("Record", this);
    recAction->setIcon(QIcon("graphics/tool/record.png"));
    connect(recAction, SIGNAL(triggered()), this, SLOT(record()));
    playbackMB->addAction(recAction);

    QAction *stopAction = new QAction("Stop", this);
    stopAction->setIcon(QIcon("graphics/tool/stop.png"));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    playbackMB->addAction(stopAction);

    playbackMB->addSeparator();

    QAction *backToBeginAction = new QAction("Back (To the start of the Song)", this);
    backToBeginAction->setIcon(QIcon("graphics/tool/back_to_begin.png"));
    connect(backToBeginAction, SIGNAL(triggered()), this, SLOT(backToBegin()));
    playbackMB->addAction(backToBeginAction);

    QAction *backAction = new QAction("Back (one Measure)", this);
    backAction->setIcon(QIcon("graphics/tool/back.png"));
    connect(backAction, SIGNAL(triggered()), this, SLOT(back()));
    playbackMB->addAction(backAction);

    QAction *forwAction = new QAction("Forward (one Measure)", this);
    forwAction->setIcon(QIcon("graphics/tool/forward.png"));
    connect(forwAction, SIGNAL(triggered()), this, SLOT(forward()));
    playbackMB->addAction(forwAction);

    // Midi
    QAction *startCommandAction = new QAction("Set Start Command", this);
    connect(startCommandAction, SIGNAL(triggered()), this,
            SLOT(setStartupCmd()));
    midiMB->addAction(startCommandAction);

    QAction *settingsAction = new QAction("Midi Settings", this);
    connect(settingsAction, SIGNAL(triggered()), this,
            SLOT(midiSettings()));
    midiMB->addAction(settingsAction);

    QAction *panicAction = new QAction("Midi panic (All Notes off)", this);
    connect(panicAction, SIGNAL(triggered()), this, SLOT(panic()));
    midiMB->addAction(panicAction);

    // Remote
    QAction *remoteDAction = new QAction("Show Remote Dialog", this);
    connect(remoteDAction, SIGNAL(triggered()), this, SLOT(showRemoteDialog()));
    remoteMB->addAction(remoteDAction);

    // Help
    QAction *manualAction = new QAction("User Manual", this);
    connect(manualAction, SIGNAL(triggered()), this, SLOT(manual()));
    helpMB->addAction(manualAction);

    QAction *aboutAction = new QAction("About", this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    helpMB->addAction(aboutAction);

    QAction *donateAction = new QAction("Donate", this);
    connect(donateAction, SIGNAL(triggered()), this, SLOT(donate()));
    helpMB->addAction(donateAction);

    QWidget *buttonBar = new QWidget(parent);
    QGridLayout *btnLayout = new QGridLayout(buttonBar);
    buttonBar->setLayout(btnLayout);
    btnLayout->setSpacing(0);
    buttonBar->setContentsMargins(0,0,0,0);
    QToolBar *fileTB = new QToolBar("File", buttonBar);

	fileTB->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    fileTB->setFloatable(false);
    fileTB->setContentsMargins(0,0,0,0);
    fileTB->layout()->setSpacing(0);
    fileTB->setIconSize(QSize(35,35));
    fileTB->addAction(newAction);
    fileTB->setStyleSheet("QToolBar { border: 0px }");
    QAction *loadAction2 = new QAction("Open File...", this);
    loadAction2->setIcon(QIcon("graphics/tool/load.png"));
    connect(loadAction2, SIGNAL(triggered()), this, SLOT(load()));
    loadAction2->setMenu(_recentPathsMenu);
    fileTB->addAction(loadAction2);

    fileTB->addAction(saveAction);
    fileTB->addSeparator();

    fileTB->addAction(undoAction);
    fileTB->addAction(redoAction);

    fileTB->addSeparator();

    btnLayout->addWidget(fileTB, 0, 0, 2, 1);


    QToolBar *upperTB = new QToolBar(buttonBar);
    QToolBar *lowerTB = new QToolBar(buttonBar);
    btnLayout->addWidget(upperTB, 0, 1, 1, 1);
    btnLayout->addWidget(lowerTB, 1, 1, 1, 1);
    upperTB->setFloatable(false);
    upperTB->setContentsMargins(0,0,0,0);
    upperTB->layout()->setSpacing(0);
	upperTB->setIconSize(QSize(20,20));
    lowerTB->setFloatable(false);
    lowerTB->setContentsMargins(0,0,0,0);
    lowerTB->layout()->setSpacing(0);
	lowerTB->setIconSize(QSize(20,20));
	lowerTB->setStyleSheet("QToolBar { border: 0px }");
	upperTB->setStyleSheet("QToolBar { border: 0px }");

	lowerTB->addAction(copyAction);
	lowerTB->addAction(pasteAction);
	lowerTB->addSeparator();

    lowerTB->addAction(zoomHorInAction);
    lowerTB->addAction(zoomHorOutAction);
    lowerTB->addAction(zoomVerInAction);
    lowerTB->addAction(zoomVerOutAction);

    lowerTB->addSeparator();

	lowerTB->addAction(backToBeginAction);
    lowerTB->addAction(backAction);
    lowerTB->addAction(playAction);
    lowerTB->addAction(pauseAction);
    lowerTB->addAction(stopAction);
    lowerTB->addAction(recAction);
	lowerTB->addAction(forwAction);
	lowerTB->addSeparator();

	_lockAction = new QAction("Lock screen while playing", this);
	_lockAction->setIcon(QIcon("graphics/tool/screen_unlocked.png"));
	lowerTB->addAction(_lockAction);
	connect(_lockAction, SIGNAL(triggered()), this, SLOT(toggleScreenLock()));

    StandardTool *tool = new StandardTool();
    Tool::setCurrentTool(tool);

    upperTB->addAction(new ToolButton(tool));
	tool->buttonClick();
    upperTB->addAction(new ToolButton(new SelectTool(SELECTION_TYPE_SINGLE)));
    upperTB->addAction(new ToolButton(new SelectTool(SELECTION_TYPE_BOX)));
    upperTB->addAction(new ToolButton(new SelectTool(SELECTION_TYPE_LEFT)));
    upperTB->addAction(new ToolButton(new SelectTool(SELECTION_TYPE_RIGHT)));

    upperTB->addSeparator();

    upperTB->addAction(new ToolButton(new EventMoveTool(true, true)));
    upperTB->addAction(new ToolButton(new EventMoveTool(false, true)));
    upperTB->addAction(new ToolButton(new EventMoveTool(true, false)));
    upperTB->addAction(new ToolButton(new SizeChangeTool()));

    upperTB->addSeparator();


	upperTB->addAction(alignLeftAction);
	upperTB->addAction(alignRightAction);
	upperTB->addAction(equalizeAction);
	upperTB->addSeparator();

    upperTB->addAction(new ToolButton(new NewNoteTool()));
    upperTB->addAction(new ToolButton(new EraserTool()));

   // upperTB->addSeparator();

    btnLayout->setColumnStretch(3, 1);

    return buttonBar;
}
