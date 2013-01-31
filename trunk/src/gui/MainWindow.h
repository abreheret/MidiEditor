/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QScrollBar>
#include <QCloseEvent>
#include <QSettings>

class MatrixWidget;
class MidiEvent;
class MidiFile;
class ChannelListWidget;
class ProtocolWidget;
class VelocityWidget;
class EventWidget;
class ClickButton;
class QStringList;
class QTabWidget;
class QMenu;
class TrackListWidget;
class QComboBox;
class RemoteServer;
class RemoteDialog;

class MainWindow : public QMainWindow {

	Q_OBJECT

	public:
		MainWindow();
		void setFile(MidiFile *f);
		EventWidget *eventWidget();
		void setStartDir(QString dir);

	public slots:
		void matrixSizeChanged(int maxScrollTime, int maxScrollLine, int vX, int vY);
		void play();
		void stop(bool autoConfirmRecord = false, bool addEvents = true, bool resetPause = true);
		void pause();
		void forward();
		void back();
		void load();
		void openFile(QString filePath);
		void save();
		void saveas();
		void undo();
		void redo();
		void muteAllChannels();
		void unmuteAllChannels();
		void allChannelsVisible();
		void allChannelsInvisible();
		void muteAllTracks();
		void unmuteAllTracks();
		void allTracksVisible();
		void allTracksInvisible();
		void donate();
		void about();
		void showRemoteDialog();
		void setFileLengthMs();
		void scrollPositionsChanged(int startMs, int maxMs, int startLine,
				int maxLine);
		void setStartupCmd();
		void midiSettings();
		void record();
		void newFile();
		void panic();
		void toggleScreenLock();
		void scaleSelection();
		void alignLeft();
		void alignRight();
		void equalize();
		void deleteSelectedEvents();
		void deleteChannel(QAction *action);
		void moveSelectedEventsToChannel(QAction *action);
		void moveSelectedEventsToTrack(QAction *action);
		void updateRecentPathsList();
		void openRecent(QAction *action);
		void updateChannelMenu();
		void updateTrackMenu();
		void muteChannel(QAction *action);
		void soloChannel(QAction *action);
		void viewChannel(QAction *action);
		void instrumentChannel(QAction *action);

		void renameTrackMenuClicked(QAction *action);
		void removeTrackMenuClicked(QAction *action);
		void showEventWidget(MidiEvent *event);
		void showTrackMenuClicked(QAction *action);
		void muteTrackMenuClicked(QAction *action);

		void renameTrack(int tracknumber);
		void removeTrack(int tracknumber);

		void setInstrumentForChannel(int i);
		void spreadSelection();
		void copy();
		void paste();

		void addTrack();

		void selectAll();

		void transposeNSemitones();

		void markEdited();

		void colorsByChannel();
		void colorsByTrack();

		void editChannel(int i);
		void editTrack(int i);
	protected:
		void closeEvent(QCloseEvent *event);
		void keyPressEvent(QKeyEvent* e);
		void keyReleaseEvent(QKeyEvent *event);

	private:
		MatrixWidget *mw_matrixWidget;
		QScrollBar *vert, *hori;
		ChannelListWidget *channelWidget;
		ProtocolWidget *protocolWidget;
		VelocityWidget *mw_velocityWidget;
		TrackListWidget *_trackWidget;
		MidiFile *file;
		QString startDirectory;
		EventWidget *_eventWidget;
		QSettings *_settings;
		ClickButton *_lockButton;
		QStringList _recentFilePaths;
		QMenu *_recentPathsMenu, *_channelVisibilityMenu, *_channelMuteMenu, *_channelIntrumentMenu,
			*_channelSoloMenu, *_renameTrackMenu, *_removeTrackMenu, *_moveSelectedEventsToTrackMenu,
			*_trackVisibilityMenu, *_trackMuteMenu;
		QTabWidget *lowerTabWidget;
		QAction *_colorsByChannel, *_colorsByTracks;

		QComboBox *_chooseEditTrack, *_chooseEditChannel;
		RemoteServer *_remoteServer;
};

#endif
