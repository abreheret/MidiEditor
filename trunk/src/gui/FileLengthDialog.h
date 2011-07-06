#ifndef FILELENGTHDIALOG_H_
#define FILELENGTHDIALOG_H_

#include <QDialog>

class MidiFile;
class QSpinBox;

class FileLengthDialog : public QDialog {

	public:
		FileLengthDialog(MidiFile *f, QWidget *parent=0);

	public slots:
		void accept();

	private:
		MidiFile *_file;
		QSpinBox *_box;
};

#endif
