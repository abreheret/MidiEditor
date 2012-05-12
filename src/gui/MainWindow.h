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
class MidiFile;
class ChannelListWidget;
class ProtocolWidget;
class VelocityWidget;
class EventWidget;
class ClickButton;

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
		void stop();
		void forward();
		void back();
		void load();
		void save();
		void saveas();
		void undo();
		void redo();
		void muteAllChannels();
		void unmuteAllChannels();
		void allChannelsVisible();
		void allChannelsInvisible();
		void donate();
		void about();
		void setFileLengthMs();
		void scrollPositionsChanged(int startMs, int maxMs, int startLine,
				int maxLine);
		void setStartupCmd();
		void midiSettings();
		void record();
		void stopRecord();
		void newFile();
		void panic();
		void toggleScreenLock();
		void scaleSelection();
		void alignLeft();
		void alignRight();
		void equalize();
		void deleteSelectedEvents();

	protected:
		void closeEvent(QCloseEvent *event);

	private:
		MatrixWidget *mw_matrixWidget;
		QScrollBar *vert, *hori;
		ChannelListWidget *channelWidget;
		ProtocolWidget *protocolWidget;
		VelocityWidget *mw_velocityWidget;
		MidiFile *file;
		QString startDirectory;
		EventWidget *_eventWidget;
		QSettings *_settings;
		ClickButton *_lockButton;
};

#endif
