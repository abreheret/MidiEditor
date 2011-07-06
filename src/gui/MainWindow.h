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
