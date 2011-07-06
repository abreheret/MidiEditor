#ifndef DONATEDIALOG_H_
#define DONATEDIALOG_H_

#include <QDialog>
#include <QObject>
#include <QWidget>

class DonateDialog : public QDialog {

	Q_OBJECT

	public:
		DonateDialog(QWidget *parent=0);
};

#endif
