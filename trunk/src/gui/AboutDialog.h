#ifndef ABOUTDIALOG_H_
#define ABOUTDIALOG_H_

#include <QDialog>
#include <QObject>
#include <QWidget>

class AboutDialog : public QDialog {

	Q_OBJECT

	public:
		AboutDialog(QWidget *parent=0);
};

#endif
