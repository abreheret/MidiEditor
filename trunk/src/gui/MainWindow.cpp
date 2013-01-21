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
#include "MatrixWidget.h"
#include "VelocityWidget.h"
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
#include <QInputDialog>
#include "../MidiEvent/OnEvent.h"
#include "TransposeDialog.h"
#include "../MidiEvent/NoteOnEvent.h"

MainWindow::MainWindow() : QMainWindow() {

	file = 0;

	_settings = new QSettings(QString("MidiEditor"), QString("NONE"));

	startDirectory = QDir::homePath();

	if(_settings->value("open_path").toString()!=""){
		startDirectory = _settings->value("open_path").toString();
	} else {
		_settings->setValue("open_path", startDirectory);
	}

	// read recent paths
	_recentFilePaths = _settings->value("recent_file_list").toStringList();

	EditorTool::setMainWindow(this);

	setWindowTitle("MidiEditor 2.0.0");
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
	connect(channelWidget, SIGNAL(channelStateChanged()), this, SLOT(updateChannelMenu()));
	connect(channelWidget, SIGNAL(selectInstrumentClicked(int)), this, SLOT(setInstrumentForChannel(int)));
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

	// Buttons
	QToolBar *buttons = new QToolBar(central);

	ClickButton *newFile = new ClickButton("new.png");
	newFile->setToolTip("Create new MidiFile");
	buttons->addWidget(newFile);
	connect(newFile, SIGNAL(clicked()), this, SLOT(newFile()));

	buttons->addSeparator();

	ClickButton *load = new ClickButton("load.png");
	load->setToolTip("Open MidiFile...");
	buttons->addWidget(load);
	connect(load, SIGNAL(clicked()), this, SLOT(load()));

	buttons->addSeparator();

	ClickButton *save = new ClickButton("save.png");
	save->setToolTip("Save file");
	buttons->addWidget(save);
	connect(save, SIGNAL(clicked()), this, SLOT(save()));

	ClickButton *saveas = new ClickButton("saveas.png");
	saveas->setToolTip("Save file as...");
	buttons->addWidget(saveas);
	connect(saveas, SIGNAL(clicked()), this, SLOT(saveas()));

	buttons->addSeparator();

	ClickButton *undo = new ClickButton("undo.png");
	undo->setToolTip("Undo");
	buttons->addWidget(undo);
	connect(undo, SIGNAL(clicked()), this, SLOT(undo()));

	ClickButton *redo = new ClickButton("redo.png");
	redo->setToolTip("Redo");
	buttons->addWidget(redo);
	connect(redo, SIGNAL(clicked()), this, SLOT(redo()));

	buttons->addSeparator();

	StandardTool *tool = new StandardTool();
	Tool::setCurrentTool(tool);

	buttons->addWidget(new ToolButton(tool));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_SINGLE)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_BOX)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_LEFT)));
	buttons->addWidget(new ToolButton(new SelectTool(SELECTION_TYPE_RIGHT)));

	buttons->addSeparator();

	buttons->addWidget(new ToolButton(new EventMoveTool(true, true)));
	buttons->addWidget(new ToolButton(new EventMoveTool(false, true)));
	buttons->addWidget(new ToolButton(new EventMoveTool(true, false)));
	buttons->addWidget(new ToolButton(new SizeChangeTool()));

	buttons->addSeparator();

	ClickButton *alignLeft = new ClickButton("align_left.png");
	alignLeft->setToolTip("Align to leftmost");
	buttons->addWidget(alignLeft);
	connect(alignLeft, SIGNAL(clicked()), this, SLOT(alignLeft()));

	ClickButton *alignRight = new ClickButton("align_right.png");
	alignLeft->setToolTip("Align to rightmost");
	buttons->addWidget(alignRight);
	connect(alignRight, SIGNAL(clicked()), this, SLOT(alignRight()));

	ClickButton *equalize = new ClickButton("equalize.png");
	equalize->setToolTip("Equalize selection");
	buttons->addWidget(equalize);
	connect(equalize, SIGNAL(clicked()), this, SLOT(equalize()));

	buttons->addSeparator();

	buttons->addWidget(new ToolButton(new NewNoteTool()));
	buttons->addWidget(new ToolButton(new EraserTool()));

	buttons->addSeparator();

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

	buttons->addSeparator();

	ClickButton *record = new ClickButton("record.png");
	buttons->addWidget(record);
	record->setToolTip("Record from the selected Midi-input");
	connect(record, SIGNAL(clicked()), this, SLOT(record()));

	buttons->addSeparator();

	ClickButton *back = new ClickButton("back.png");
	buttons->addWidget(back);
	back->setToolTip("Set the cursor to the beginning of this File");
	connect(back, SIGNAL(clicked()), this, SLOT(back()));

	ClickButton *play = new ClickButton("play.png");
	buttons->addWidget(play);
	play->setToolTip("Play from the current Cursor Position");
	connect(play, SIGNAL(clicked()), this, SLOT(play()));

	ClickButton *stop = new ClickButton("stop.png");
	stop->setToolTip("Stop Playback/Record");
	buttons->addWidget(stop);
	connect(stop, SIGNAL(clicked()), this, SLOT(stop()));

	buttons->addSeparator();

	_lockButton = new ClickButton("screen_unlocked.png");
	buttons->addWidget(_lockButton);
	_lockButton->setToolTip("Do not scroll automatically while playing/recording");
	connect(_lockButton, SIGNAL(clicked()), this, SLOT(toggleScreenLock()));

	buttons->addSeparator();

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
	QMenu *tracksMB = menuBar()->addMenu("Tracks");
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

	_recentPathsMenu = new QMenu("Open recent..", this);
	fileMB->addMenu(_recentPathsMenu);
	connect(_recentPathsMenu, SIGNAL(triggered(QAction*)), this, SLOT(openRecent(QAction*)));

	updateRecentPathsList();

	fileMB->addSeparator();

	QAction *saveAction = new QAction("Save File", this);
	saveAction->setIcon(QIcon("graphics/tool/save.png"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
	fileMB->addAction(saveAction);

	QAction *saveAsAction = new QAction("Save As...", this);
	saveAsAction->setIcon(QIcon("graphics/tool/saveas.png"));
	connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveas()));
	fileMB->addAction(saveAsAction);

	fileMB->addSeparator();

	QAction *quitAction = new QAction("Quit", this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
	fileMB->addAction(quitAction);

	// Edit

	QAction *undoAction = new QAction("Undo", this);
	undoAction->setIcon(QIcon("graphics/tool/undo.png"));
	connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
	editMB->addAction(undoAction);

	QAction *redoAction = new QAction("Redo", this);
	redoAction->setIcon(QIcon("graphics/tool/redo.png"));
	connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
	editMB->addAction(redoAction);

	editMB->addSeparator();

	QAction *selectAllAction = new QAction("Select all Events", this);
	connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
	editMB->addAction(selectAllAction);


	editMB->addSeparator();

	QAction *copyAction = new QAction("Copy Events to Clipboard", this);
	connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	editMB->addAction(copyAction);

	QAction *pasteAction = new QAction("Paste Events, beginning at the Cursorposition", this);
	connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	editMB->addAction(pasteAction);

	editMB->addSeparator();

	QAction *deleteAction = new QAction("Remove selected Events", this);
	deleteAction->setIcon(QIcon("graphics/tool/eraser.png"));
	connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSelectedEvents()));
	editMB->addAction(deleteAction);

	editMB->addSeparator();

	QAction *alignLeftAction = new QAction("Align events to leftmost", this);
	alignLeftAction->setIcon(QIcon("graphics/tool/align_left.png"));
	connect(alignLeftAction, SIGNAL(triggered()), this, SLOT(alignLeft()));
	editMB->addAction(alignLeftAction);

	QAction *alignRightAction = new QAction("Align events to rightmost", this);
	alignRightAction->setIcon(QIcon("graphics/tool/align_right.png"));
	connect(alignRightAction, SIGNAL(triggered()), this, SLOT(alignRight()));
	editMB->addAction(alignRightAction);

	QAction *equalizeAction = new QAction("Equalize selection", this);
	equalizeAction->setIcon(QIcon("graphics/tool/equalize.png"));
	connect(equalizeAction, SIGNAL(triggered()), this, SLOT(equalize()));
	editMB->addAction(equalizeAction);

	editMB->addSeparator();

	QMenu *moveToChannelMenu = new QMenu("Move selected Events to Channel...", editMB);
	editMB->addMenu(moveToChannelMenu);
	connect(moveToChannelMenu, SIGNAL(triggered(QAction*)), this, SLOT(moveSelectedEventsToChannel(QAction*)));

	for(int i = 0; i<16; i++){
		QVariant variant(i);
		QAction *moveToChannelAction = new QAction(QString::number(i), this);
		moveToChannelAction->setData(variant);
		moveToChannelMenu->addAction(moveToChannelAction);
	}

	_moveSelectedEventsToTrackMenu = new QMenu("Move selected Events to Track...", editMB);
	editMB->addMenu(_moveSelectedEventsToTrackMenu);
	connect(_moveSelectedEventsToTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(moveSelectedEventsToTrack(QAction*)));

	editMB->addSeparator();

	QAction *transposeAction = new QAction("Transpose...", this);
	connect(transposeAction, SIGNAL(triggered()), this, SLOT(transposeNSemitones()));
	editMB->addAction(transposeAction);

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

	channelsMB->addSeparator();

	QMenu *deleteChannelMenu = new QMenu("Remove all events from channel...", channelsMB);
	channelsMB->addMenu(deleteChannelMenu);
	connect(deleteChannelMenu, SIGNAL(triggered(QAction*)), this, SLOT( deleteChannel(QAction*)));

	for(int i = 0; i<16; i++){
		QVariant variant(i);
		QAction *delChannelAction = new QAction(QString::number(i), this);
		delChannelAction->setData(variant);
		deleteChannelMenu->addAction(delChannelAction);
	}

	channelsMB->addSeparator();

	_channelVisibilityMenu = new QMenu("Visible channels...", channelsMB);
	channelsMB->addMenu(_channelVisibilityMenu);
	connect(_channelVisibilityMenu, SIGNAL(triggered(QAction*)), this, SLOT(viewChannel(QAction*)));

	for(int i = 0; i<17; i++){
		QVariant variant(i);
		QAction *viewChannelAction = new QAction(QString::number(i), this);
		viewChannelAction->setCheckable(true);
		viewChannelAction->setData(variant);
		_channelVisibilityMenu->addAction(viewChannelAction);
	}

	_channelMuteMenu = new QMenu("Mute channels...", channelsMB);
	channelsMB->addMenu(_channelMuteMenu);
	connect(_channelMuteMenu, SIGNAL(triggered(QAction*)), this, SLOT(muteChannel(QAction*)));

	for(int i = 0; i<16; i++){
		QVariant variant(i);
		QAction *muteChannelAction = new QAction(QString::number(i), this);
		muteChannelAction->setCheckable(true);
		muteChannelAction->setData(variant);
		_channelMuteMenu->addAction(muteChannelAction);
	}

	_channelSoloMenu = new QMenu("Select solo channel...", channelsMB);
	channelsMB->addMenu(_channelSoloMenu);
	connect(_channelSoloMenu, SIGNAL(triggered(QAction*)), this, SLOT(soloChannel(QAction*)));

	for(int i = 0; i<16; i++){
		QVariant variant(i);
		QAction *soloChannelAction = new QAction(QString::number(i), this);
		soloChannelAction->setCheckable(true);
		soloChannelAction->setData(variant);
		_channelSoloMenu->addAction(soloChannelAction);
	}

	_channelIntrumentMenu = new QMenu("Select Intrument for channel...");
	channelsMB->addMenu(_channelIntrumentMenu);
	connect(_channelIntrumentMenu, SIGNAL(triggered(QAction*)), this, SLOT(instrumentChannel(QAction*)));

	for(int i = 0; i<16; i++){
		QVariant variant(i);
		QAction *instrumentChannelAction = new QAction(QString::number(i), this);
		instrumentChannelAction->setData(variant);
		_channelIntrumentMenu->addAction(instrumentChannelAction);
	}

	// Tracks
	QAction *addTrackAction = new QAction("Add Track", tracksMB);
	tracksMB->addAction(addTrackAction);
	connect(addTrackAction, SIGNAL(triggered()), this, SLOT(addTrack()));

	tracksMB->addSeparator();

	QAction *allTracksVisible = new QAction("All Tracks visible", this);
	allTracksVisible->setIcon(QIcon("graphics/trackwidget/visible.png"));
	connect(allTracksVisible, SIGNAL(triggered()), this,
			SLOT(allTracksVisible()));
	tracksMB->addAction(allTracksVisible);

	QAction *allTracksInvisible = new QAction("All Tracks invisible", this);
	allTracksInvisible->setIcon(QIcon("graphics/trackwidget/hidden.png"));
	connect(allTracksInvisible, SIGNAL(triggered()), this,
			SLOT(allTracksInvisible()));
	tracksMB->addAction(allTracksInvisible);

	_trackVisibilityMenu = new QMenu("Show Track...", tracksMB);
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

	_trackMuteMenu = new QMenu("Mute Track...", tracksMB);
	tracksMB->addMenu(_trackMuteMenu);
	connect(_trackMuteMenu, SIGNAL(triggered(QAction*)), this, SLOT(muteTrackMenuClicked(QAction*)));

	tracksMB->addSeparator();

	_renameTrackMenu = new QMenu("Rename Track...", tracksMB);
	tracksMB->addMenu(_renameTrackMenu);
	connect(_renameTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(renameTrackMenuClicked(QAction*)));

	_removeTrackMenu = new QMenu("Remove Track...", tracksMB);
	tracksMB->addMenu(_removeTrackMenu);
	connect(_removeTrackMenu, SIGNAL(triggered(QAction*)), this, SLOT(removeTrackMenuClicked(QAction*)));

	// Timing
	QAction *setFileLengthMs = new QAction("Set File Length (ms)", this);
	connect(setFileLengthMs, SIGNAL(triggered()), this,SLOT(setFileLengthMs()));
	timingMB->addAction(setFileLengthMs);

	QAction *scaleSelection = new QAction("Scale selected events", this);
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

	QMenu *colorMenu = new QMenu("Set Event colors...", tracksMB);
	_colorsByChannel = new QAction("by channels", this);
	_colorsByChannel->setCheckable(true);
	connect(_colorsByChannel, SIGNAL(triggered()), this,SLOT(colorsByChannel()));
	colorMenu->addAction(_colorsByChannel);

	_colorsByTracks = new QAction("by tracks", this);
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

	QAction *stopAction = new QAction("Stop", this);
	stopAction->setIcon(QIcon("graphics/tool/stop.png"));
	connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
	playbackMB->addAction(stopAction);

	playbackMB->addSeparator();

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

	NewNoteTool::setEditTrack(0);
	NewNoteTool::setEditChannel(0);

	protocolWidget->setFile(file);
	channelWidget->setFile(file);
	_trackWidget->setFile(file);
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

		mw_velocityWidget->setEnabled(false);
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
	}
}

void MainWindow::stop(){
	if(!MidiInput::recording() && MidiPlayer::isPlaying()){
		MidiPlayer::stop();
		mw_velocityWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		_trackWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);
		mw_matrixWidget->timeMsChanged(MidiPlayer::timeMs(), true);
		_trackWidget->setEnabled(true);
		panic();
	}

	if(MidiInput::recording()){
		MidiPlayer::stop();
		panic();
		mw_velocityWidget->setEnabled(true);
		channelWidget->setEnabled(true);
		protocolWidget->setEnabled(true);
		mw_matrixWidget->setEnabled(true);
		_trackWidget->setEnabled(true);

		QMultiMap<int, MidiEvent*> events = MidiInput::endInput();

		RecordDialog *dialog = new RecordDialog(file, events, this);
		dialog->setModal(true);
		dialog->show();
	}
}

void MainWindow::forward(){
	if(!file) return;
	file->setCursorTick(file->endTick());
	mw_matrixWidget->update();
}

void MainWindow::back(){
	if(!file) return;
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

void MainWindow::record(){

	if(!file){
		newFile();
	}

	if(!MidiInput::recording() && !MidiPlayer::isPlaying()){
		// play current file
		if(file){

			mw_matrixWidget->timeMsChanged(file->msOfTick(file->cursorTick()), true);

			mw_velocityWidget->setEnabled(false);
			channelWidget->setEnabled(false);
			protocolWidget->setEnabled(false);
			mw_matrixWidget->setEnabled(false);
			_trackWidget->setEnabled(false);

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

void MainWindow::scaleSelection(){
    bool ok;
    double scale = QInputDialog::getDouble(this, "Scalefactor",
    		"Scalefactor:", 1.0, 0, 2147483647, 1, &ok);
    qWarning("scale %d events", EventTool::selectedEventList()->size());
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
    if (EventTool::selectedEventList()->size()>0 && file){
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

	openFile(path);
}


void MainWindow::updateChannelMenu() {


	// visibilitymenu
	foreach(QAction *action, _channelVisibilityMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->visible());
		} else {
			action->setChecked(false);
		}
	}

	// mute menu
	foreach(QAction *action, _channelMuteMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->mute());
		} else {
			action->setChecked(false);
		}
	}

	// solo menu
	foreach(QAction *action, _channelSoloMenu->actions()){
		int channel = action->data().toInt();
		if(file){
			action->setChecked(file->channel(channel)->solo());
		} else {
			action->setChecked(false);
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
			file->channel(i)->setSolo(i==channel);
			updateChannelMenu();
			channelWidget->update();
		}
		file->protocol()->endAction();
	}
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
		file->protocol()->startNewAction("Add Track");
		file->addTrack();
		file->protocol()->endAction();
	}
	updateTrackMenu();
	if(file){
		renameTrack(file->numTracks()-1);
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
	mw_velocityWidget->update();
}
void MainWindow::colorsByTrack(){
	mw_matrixWidget->setColorsByTracks();
	_colorsByChannel->setChecked(false);
	_colorsByTracks->setChecked(true);
	mw_matrixWidget->registerRelayout();
	mw_matrixWidget->update();
	mw_velocityWidget->update();
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
	d->show();
}

void MainWindow::instrumentChannel(QAction *action){
	if(file){
		setInstrumentForChannel(action->data().toInt());
	}
}
