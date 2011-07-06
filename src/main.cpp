#include <QApplication>

#include "gui/MainWindow.h"
#include "midi/MidiOutput.h"
#include "midi/MidiInput.h"

#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	MidiOutput::init();
    MidiInput::init();

    MainWindow *w = new MainWindow();
    w->showMaximized();

    return a.exec();
}
