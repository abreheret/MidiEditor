#ifndef MIDISETTINGSDIALOG_H_
#define MIDISETTINGSDIALOG_H_

#include <QDialog>
#include <QList>

class QWidget;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

class MidiSettingsDialog : public QDialog {

	Q_OBJECT

	public:

		/**
		 * \brief Create a new MidiSettingsDialog.
		 */
		MidiSettingsDialog(QWidget *parent = 0);

	public slots:

		/**
		 * \brief reloads the input ports
		 */
		void reloadInputPorts();

		/**
		 * \brief reloads the output ports
		 */
		void reloadOutputPorts();

		/**
		 * \brief called when an inputItem has been checked.
		 */
		void inputChanged(QListWidgetItem *item);

		/**
		 * \brief called when an outputItem has been checked.
		 */
		void outputChanged(QListWidgetItem *item);

	private:

		/**
		 * \brief a list of selected input/ output ports
		 */
		QStringList *_inputPorts, *_outputPorts;

		/**
		 * \brief the Lists
		 */
		QListWidget *_inList, *_outList;

		/**
		 * \brief the textfield with the startup command.
		 */
		QLineEdit *_commandEdit;
};

#endif
