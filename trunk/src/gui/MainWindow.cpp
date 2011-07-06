#include "MainWindow.h"

#include <QGridLayout>
#include "MatrixWidget.h"
#include "VelocityWidget.h"
#include "../midi/MidiFile.h"
#include "ChannelListWidget.h"
#include "../tool/Tool.h"
#include "../tool/SelectTool.h"
#include "../tool/SizeChangeTool.h"
#include "../tool/EraserTool.h"
#include "../tool/StandardTool.h"
#include "../tool/NewNoteTool.h"
#include "../tool/EventMoveTool.h"
#include "EventWidget.h"
#include "../tool/ToolButton.h"
#include <QScrollArea>
#include "../protocol/Protocol.h"
#include "ProtocolWidget.h"
#include "ClickButton.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
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
#include "../MidiEvent/OffEvent.h"
#include "RecordDialog.h"
#include <QSettings>
#include "../Terminal.h"
#include <QTextEdit>
#include <QToolBar>

MainWindow::MainWindow() : QMainWindow() {

	file = 0;

	_settings = new QSettings(QString("MidiEditor"), QString("NONE"));

	startDirectory = QDir::homePath();
	if(_settings->value("open_path").toString()!=""){
		startDirectory = _settings->value("open_path").toString();
	} else {
		_settings->setValue("open_path", startDirectory);
	}
	EditorTool::setMainWindow(this);

	setWindowTitle("MidiEditor 1.0.1");
	setWindowIcon(QIcon("graphics/icon.png"));

	QWidget *central = new QWidget(this);
	QGridLayout *centralLayout = new QGridLayout(central);
	centralLayout->setContentsMargins(3,3,3,5);

	// there is a vertical split
	QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, central);
	mainSplitter->setHandleWidth(0);

	// The left side
	QSplitter *leftSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	leftSplitter->setHandleWidth(0);
	mainSplitter->addWidget(leftSplitter);
	leftSplitter->setContentsMargins(0,0,0,0);

	// The right side
	QSplitter *rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	rightSplitter->setHandleWidth(0);
	mainSplitter->addWidget(rightSplitter);

	// Set the sizes of mainSplitter
	mainSplitter->setStretchFactor(0, 92);
	mainSplitter->setStretchFactor(1, 8);
	mainSplitter->setContentsMargins(0,0,0,0);

	// the channelWidget and the trackWidget are tabbed
	QTabWidget *upperTabWidget = new QTabWidget(rightSplitter);
	rightSplitter->addWidget(upperTabWidget);
	rightSplitter->setContentsMargins(0,0,0,0);

	// protocolList and EventWidget are tabbed
	QTabWidget *lowerTabWidget = new QTabWidget(rightSplitter);
	rightSplitter->addWidget(lowerTabWidget);

	rightSplitter->setStretchFactor(0, 15);
	rightSplitter->setStretchFactor(1, 85);

	// MatrixArea
	QWidget *matrixArea = new QWidget(leftSplitter);
	leftSplitter->addWidget(matrixArea);
	mw_matrixWidget = new MatrixWidget(matrixArea);
	vert = new QScrollBar(Qt::Vertical, matrixArea);
	QGridLayout *matrixAreaLayout = new QGridLayout(matrixArea);
	matrixAreaLayout->setContentsMargins(0,0,0,0);
	matrixAreaLayout->addWidget(mw_matrixWidget, 0, 0);
	matrixAreaLayout->addWidget(vert, 0, 1);
	matrixAreaLayout->setColumnStretch(0, 1);
	matrixArea->setLayout(matrixAreaLayout);

	// VelocityArea
	QWidget *velocityArea = new QWidget(leftSplitter);
	leftSplitter->addWidget(velocityArea);
	hori = new QScrollBar(Qt::Horizontal, velocityArea);
	hori->setSingleStep(500);
	mw_velocityWidget = new VelocityWidget(mw_matrixWidget, velocityArea);
	QGridLayout *velocityAreaLayout = new QGridLayout(velocityArea);
	velocityAreaLayout->setContentsMargins(0,0,0,0);
	velocityAreaLayout->addWidget(mw_velocityWidget, 0, 0);
	// there is a Scrollbar on the right side of the velocityWidget doing
	// nothing but making the VelocityWidget as big as the matrixWidget
	QScrollBar *scrollNothing = new QScrollBar(Qt::Vertical, velocityArea);
	scrollNothing->setMinimum(0);
	scrollNothing->setMaximum(0);
	velocityAreaLayout->addWidget(scrollNothing, 0,1,1,1);
	velocityAreaLayout->addWidget(hori, 1, 0, 1, 2);
	velocityAreaLayout->setRowStretch(0, 1);
	velocityArea->setLayout(velocityAreaLayout);

	// Set the sizes of leftSplitter
	leftSplitter->setStretchFactor(0, 9);
	leftSplitter->setStretchFactor(1, 1);

	// Channels
	QScrollArea *channelScroll = new QScrollArea(upperTabWidget);
	channelWidget = new ChannelListWidget(channelScroll);
	channelScroll->setWidget(channelWidget);
	channelScroll->setWidgetResizable(true);
	upperTabWidget->addTab(channelScroll, "Channels");

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

	// Buttons
	QToolBar *buttons = new QToolBar(central);
//	buttons->setIconSize(34);
//	QBoxLayout *box = new QBoxLayout(QBoxLayout::LeftToRight, buttons);
//	box->setContentsMargins(0,0,0,0);

	ClickButton *newFile = new ClickButton("new.png");
	newFile->setToolTip("Create new MidiFile");
	buttons->addWidget(newFile);
	connect(newFile, SIGNAL(clicked()), this, SLOT(newFile()));

	ClickButton *load = new ClickButton("load.png");
	load->setToolTip("Open MidiFile...");
	buttons->addWidget(load);
	connect(load, SIGNAL(clicked()), this, SLOT(load()));

	ClickButton *save = new ClickButton("save.png");
	save->setToolTip("Save file");
	buttons->addWidget(save);
	connect(save, SIGNAL(clicked()), this, SLOT(save()));

	ClickButton *saveas = new ClickButton("saveas.png");
	saveas->setToolTip("Save file as...");
	buttons->addWidget(saveas);
	connect(saveas, SIGNAL(clicked()), this, SLOT(saveas()));

	ClickButton *undo = new ClickButton("undo.png");
	undo->setToolTip("Undo");
	buttons->addWidget(undo);
	connect(undo, SIGNAL(clicked()), this, SLOT(undo()));

	ClickButton *redo = new ClickButton("redo.png");
	redo->setToolTip("Redo");
	buttons->addWidget(redo);
	connect(redo, SIGNAL(clicked()), this, SLOT(redo()));

	StandardTool *tool = new StandardTool();
	Tool::setCurrentTool(tool);

	buttons->addWidget(new ToolButton(tool));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_SINGLE)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_BOX)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_LEFT)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_RIGHT)));

	buttons->addWidget(new ToolButton(new EventMoveTool(true, true)));
	buttons->addWidget(new ToolButton(new EventMoveTool(false, true)));
	buttons->addWidget(new ToolButton(new EventMoveTool(true, false)));
	buttons->addWidget(new ToolButton(new SizeChangeTool()));
	buttons->addWidget(new ToolButton(new NewNoteTool()));
	buttons->addWidget(new ToolButton(new EraserTool()));

	ClickButton *zoom_ver_in = new ClickButton("zoom_ver_in.png");
	zoom_ver_in->setToolTip("Zoom vertical in");
	buttons->addWidget(zoom_ver_in);
	connect(zoom_ver_in, SIGNAL(clicked()), mw_matrixWidget, SLOT(zoomVerIn()));
	ClickButton *zoom_ver_out = new ClickButton("zoom_ver_out.png");
	zoom_ver_out->setToolTip("Zoom vertical out");
	buttons->addWidget(zoom_ver_out);
	connect(zoom_ver_out, SIGNAL(clicked()),mw_matrixWidget,SLOT(zoomVerOut()));
	ClickButton *zoom_hor_in = new ClickButton("zoom_hor_in.png");
	zoom_hor_in->setToolTip("Zoom horizontal in");
	buttons->addWidget(zoom_hor_in);
	connect(zoom_hor_in, SIGNAL(clicked()), mw_matrixWidget, SLOT(zoomHorIn()));
	ClickButton *zoom_hor_out = new ClickButton("zoom_hor_out.png");
	zoom_hor_out->setToolTip("Zoom horizontal out");
	buttons->addWidget(zoom_hor_out);
	connect(zoom_hor_out, SIGNAL(clicked()),mw_matrixWidget,SLOT(zoomHorOut()));

	ClickButton *back = new ClickButton("back.png");
	buttons->addWidget(back);
	back->setToolTip("Set the cursor to the beginning of this File");
	connect(back, SIGNAL(clicked()), this, SLOT(back()));

	ClickButton *play = new ClickButton("play.png");
	buttons->addWidget(play);
	play->setToolTip("Play from the current Cursor Position");
	connect(play, SIGNAL(clicked()), this, SLOT(play()));

	ClickButton *stop = new ClickButton("stop.png");
	stop->setToolTip("Stop Playback");
	buttons->addWidget(stop);
	connect(stop, SIGNAL(clicked()), this, SLOT(stop()));

	ClickButton *forward = new ClickButton("forward.png");
	//box->addWidget(forward);
	forward->setToolTip("Set the cursor to the end of this File");
	connect(forward, SIGNAL(clicked()), this, SLOT(forward()));

	ClickButton *record = new ClickButton("record.png");
	buttons->addWidget(record);
	record->setToolTip("Record from the selected Midi-input");
	connect(record, SIGNAL(clicked()), this, SLOT(record()));

	ClickButton *stop_record = new ClickButton("stop_record.png");
	buttons->addWidget(stop_record);
	stop_record->setToolTip("Stop recording...");
	connect(stop_record, SIGNAL(clicked()), this, SLOT(stopRecord()));

	_lockButton = new ClickButton("screen_unlocked.png");
	buttons->addWidget(_lockButton);
	_lockButton->setToolTip("Do not scroll automatically while playing/recording");
	connect(_lockButton, SIGNAL(clicked()), this, SLOT(toggleScreenLock()));

	// Add the Widgets to the central Layout
	centralLayout->addWidget(buttons,0,0);
	centralLayout->addWidget(mainSplitter,1,0);
	centralLayout->setRowStretch(1, 1);
	central->setLayout(centralLayout);

	setCentralWidget(central);

	// Menubar
	QMenu *fileMB = menuBar()->addMenu("File");
	QMenu *editMB = menuBar()->addMenu("Edit");
	QMenu *channelsMB = menuBar()->addMenu("Channels");
	QMenu *timingMB = menuBar()->addMenu("Timing");
	QMenu *viewMB = menuBar()->addMenu("View");
	QMenu *playbackMB = menuBar()->addMenu("Playback");
	QMenu *midiMB = menuBar()->addMenu("Midi");
	QMenu *helpMB = menuBar()->addMenu("Help");

	// File
	QAction *newAction = new QAction("New File", this);
	connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
	fileMB->addAction(newAction);

	QAction *loadAction = new QAction("Open File...", this);
	loadAction->setIcon(QIcon("graphics/tool/load.png"));
	connect(loadAction, SIGNAL(triggered()), this, SLOT(load()));
	fileMB->addAction(loadAction);

	QAction *saveAction = new QAction("Save File", this);
	saveAction->setIcon(QIcon("graphics/tool/save.png"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
	fileMB->addAction(saveAction);

	QAction *saveAsAction = new QAction("Save As...", this);
	saveAsAction->setIcon(QIcon("graphics/tool/saveas.png"));
	connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveas()));
	fileMB->addAction(saveAsAction);

	// Edit
	QAction *undoAction = new QAction("Undo", this);
	undoAction->setIcon(QIcon("graphics/tool/undo.png"));
	connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
	editMB->addAction(undoAction);

	QAction *redoAction = new QAction("Redo", this);
	redoAction->setIcon(QIcon("graphics/tool/redo.png"));
	connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
	editMB->addAction(redoAction);

	// channels
	QAction *allChannelsVisible = new QAction("All Channels visible", this);
	allChannelsVisible->setIcon(QIcon("graphics/channelwidget/visible.png"));
	connect(allChannelsVisible, SIGNAL(triggered()), this,
			SLOT(allChannelsVisible()));
	channelsMB->addAction(allChannelsVisible);

	QAction *allChannelsInvisible = new QAction("All channels invisible", this);
	allChannelsInvisible->setIcon(QIcon("graphics/channelwidget/hidden.png"));
	connect(allChannelsInvisible, SIGNAL(triggered()), this,
			SLOT(allChannelsInvisible()));
	channelsMB->addAction(allChannelsInvisible);

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

	// Timing
	QAction *setFileLengthMs = new QAction("Set File Length (ms)", this);
	connect(setFileLengthMs, SIGNAL(triggered()), this,SLOT(setFileLengthMs()));
	timingMB->addAction(setFileLengthMs);

	// View
	QAction *zoomHorOutAction = new QAction("Zoom out horizontal", this);
	zoomHorOutAction->setIcon(QIcon("graphics/tool/zoom_hor_out.png"));
	connect(zoomHorOutAction, SIGNAL(triggered()),
			mw_matrixWidget,SLOT(zoomHorOut()));
	viewMB->addAction(zoomHorOutAction);

	QAction *zoomHorInAction = new QAction("Zoom in horizontal", this);
	zoomHorInAction->setIcon(QIcon("graphics/tool/zoom_hor_in.png"));
	connect(zoomHorInAction, SIGNAL(triggered()),
			mw_matrixWidget,SLOT(zoomHorIn()));
	viewMB->addAction(zoomHorInAction);

	QAction *zoomVerOutAction = new QAction("Zoom out vertical", this);
	zoomVerOutAction->setIcon(QIcon("graphics/tool/zoom_ver_out.png"));
	connect(zoomVerOutAction, SIGNAL(triggered()),
			mw_matrixWidget,SLOT(zoomVerOut()));
	viewMB->addAction(zoomVerOutAction);

	QAction *zoomVerInAction = new QAction("Zoom in vertical", this);
	zoomVerInAction->setIcon(QIcon("graphics/tool/zoom_ver_in.png"));
	connect(zoomVerInAction, SIGNAL(triggered()),
			mw_matrixWidget,SLOT(zoomVerIn()));
	viewMB->addAction(zoomVerInAction);

	// Playback
	QAction *playAction = new QAction("Play", this);
	playAction->setIcon(QIcon("graphics/tool/play.png"));
	connect(playAction, SIGNAL(triggered()), this, SLOT(play()));
	playbackMB->addAction(playAction);

	QAction *stopAction = new QAction("Stop", this);
	stopAction->setIcon(QIcon("graphics/tool/stop.png"));
	connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
	playbackMB->addAction(stopAction);

	QAction *forwardAction = new QAction("Set Cursor to the end of the File",
			this);
	forwardAction->setIcon(QIcon("graphics/tool/forward.png"));
	connect(forwardAction, SIGNAL(triggered()), this, SLOT(forward()));
	playbackMB->addAction(forwardAction);

	QAction *backAction = new QAction("Set Cursor to the beginning of the File",
			this);
	backAction->setIcon(QIcon("graphics/tool/back.png"));
	connect(backAction, SIGNAL(triggered()), this, SLOT(back()));
	playbackMB->addAction(backAction);

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

	// Help
	QAction *aboutAction = new QAction("About", this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	helpMB->addAction(aboutAction);

	QAction *donateAction = new QAction("Donate", this);
	connect(donateAction, SIGNAL(triggered()), this, SLOT(donate()));
	helpMB->addAction(donateAction);
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

	protocolWidget->setFile(file);
	channelWidget->setFile(file);
	Tool::setFile(file);
	this->file = file;
	setWindowTitle("MidiEditor - " +file->path());
	connect(file,SIGNAL(cursorPositionChanged()),channelWidget,SLOT(update()));
	connect(file,SIGNAL(recalcWidgetSize()),mw_matrixWidget,SLOT(calcSizes()));

	mw_matrixWidget->setFile(file);
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
		mw_velocityWidget->setEnabled(false);
		channelWidget->setEnabled(false);
		protocolWidget->setEnabled(false);
		mw_matrixWidget->setEnabled(false);

	    MidiPlayer::play(file);
		connect(MidiPlayer::playerThread(),
				SIGNAL(playerStopped()), this,	SLOT(stop()));
	}
}

void MainWindow::stop(){
	if(!MidiInput::recording() && MidiPlayer::isPlaying()){
		MidiPlayer::stop();
		mw_velocityWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);
		panic();
	}
}

void MainWindow::forward(){
	if(!file) return;
	file->setCursorTick(file->endTick());
}

void MainWindow::back(){
	if(!file) return;
	file->setCursorTick(0);
}

void MainWindow::save(){

	if(!file) return;

	if(QFile(file->path()).exists()){
		file->save(file->path());
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

	if(file->save(newPath)){
		file->setPath(newPath);
		setWindowTitle("MidiEditor - " +file->path());
	}
}

void MainWindow::load(){
	QString oldPath = startDirectory;
	if(file){
		oldPath = file->path();
		if(!file->saved()){
			switch(QMessageBox::question(this, "Save file?", "save file "+
					file->path()+
				" before closing?", "Save","Close without saving", "Break",0,2))
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
			dir);

	bool ok = true;

	QFile nf(newPath);

	if(!nf.exists()) return;

	startDirectory = QFileInfo(nf).absoluteDir().path()+"/";

	MidiFile *mf = new MidiFile(newPath, &ok);

	if(ok){
		setFile(mf);
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
			" before closing?", "Save","Close without saving", "Break",0,2))
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

	// save the ports
	QString inPort =  MidiInput::inputPort().section(':', 0, 0);
	// perhaps it has brackets with port
	if(inPort.contains('(')){
		inPort = inPort.section('(', 0, 0);
	}
	_settings->setValue("in_port", inPort);

	QString outPort =  MidiOutput::outputPort().section(':', 0, 0);
	// perhaps it has brackets with port
	if(outPort.contains('(')){
		outPort = outPort.section('(', 0, 0);
	}
	_settings->setValue("out_port", outPort);

	// save the current Path
	_settings->setValue("open_path", startDirectory);
}

void MainWindow::donate(){
	DonateDialog *d = new DonateDialog(this);
	d->show();
}

void MainWindow::about(){
	AboutDialog *d = new AboutDialog(this);
	d->show();
}

void MainWindow::setFileLengthMs(){
	if(!file) return;

	FileLengthDialog *d = new FileLengthDialog(file, this);
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
	d->show();
}

void MainWindow::record(){

	if(!MidiInput::recording() && !MidiPlayer::isPlaying()){
		// play current file
		if(file){
			mw_velocityWidget->setEnabled(false);
			channelWidget->setEnabled(false);
			protocolWidget->setEnabled(false);
			mw_matrixWidget->setEnabled(false);

			MidiPlayer::play(file);
			MidiInput::startInput();
			connect(MidiPlayer::playerThread(),
					SIGNAL(playerStopped()), this,	SLOT(stop()));
		}
	}
}

void MainWindow::stopRecord(){

	if(MidiInput::recording()){
		MidiPlayer::stop();
		panic();
		mw_velocityWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);

		QMultiMap<int, MidiEvent*> events = MidiInput::endInput();

		RecordDialog *dialog = new RecordDialog(file, events, this);
		dialog->show();
	}
}

void MainWindow::newFile(){
	if(file){
		if(!file->saved()){
			switch(QMessageBox::question(this, "Save file?", "save file "+
					file->path()+
				" before closing?", "Save","Close without saving", "Break",0,2))
			{
				case 0: {
					// save
					if(QFile(file->path()).exists()){
						file->save(file->path());
					} else {
						saveas();
					}
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
}

void MainWindow::panic(){
	MidiPlayer::panic();
}

void MainWindow::toggleScreenLock() {
	if(mw_matrixWidget->screenLocked()){
		_lockButton->setImageName("screen_unlocked.png");
		_lockButton->setToolTip("Do not scroll automatically while playing/recording");
		mw_matrixWidget->setScreenLocked(false);
	} else {
		_lockButton->setImageName("screen_locked.png");
		_lockButton->setToolTip("Scroll automatically while playing/recording");
		mw_matrixWidget->setScreenLocked(true);
	}
}
