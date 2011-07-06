#ifndef RECORDDIALOG_H_
#define RECORDDIALOG_H_

#include <QDialog>
#include <QMultiMap>

class MidiFile;
class MidiEvent;
class QCheckBox;
class QComboBox;

class RecordDialog : public QDialog {

	Q_OBJECT

	public:
		RecordDialog(MidiFile *file, QMultiMap<int, MidiEvent*> data,
				QWidget *parent = 0);

	public slots:
		void enter();
		void cancel();

	private:
		MidiFile *_file;
		QMultiMap<int, MidiEvent*> _data;
		QComboBox *_channelBox;
		QComboBox *_trackBox;
		QCheckBox *_notes, *_channelPressure, *_controlChange, *_keyPressure,
			*_progChange, *_tempoChange, *_timeSig, *_unknown;

};

#endif
