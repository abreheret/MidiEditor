#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <QObject>

class QProcess;
class QTextEdit;

class Terminal : public QObject {

	Q_OBJECT

	public:

		Terminal();

		/**
		 * \brief creates the terminal.
		 *
		 * tries to start a process with the given startstring - on success
		 * it will try to open the ports starting with the given values.
		 */
		static void initTerminal(QString startString, QString inPort,
				QString outPort);

		/**
		 * \brief returns the terminal
		 */
		static Terminal *terminal();

		/**
		 * \brief writes message to the terminal.
		 */
		void writeString(QString message);

		/**
		 * \brief executes the given string.
		 *
		 * Will connect to the ports starting with the given values if
		 * they are not empty.
		 * Will stop the current process.
		 */
		void execute(QString startString, QString inPort,
				QString outPort);

		/**
		 * \the console.
		 */
		QTextEdit *console();

	public slots:

		/**
		 * \brief Called, when a process has been finished.
		 */
		void processStarted();

		/**
		 * \brief print text from the process to the terminal.
		 */
		void printToTerminal();

		/**
		 * \brief print error from the process to the terminal.
		 */
		void printErrorToTerminal();

	private:

		static Terminal *_terminal;

		QProcess *_process;

		QTextEdit *_textEdit;

		QString _inPort, _outPort;
};

#endif
