TEMPLATE = app
TARGET = MidiEditor
QT += core
HEADERS += src/midi/SenderThread.h \
    src/Terminal.h \
    src/gui/RecordDialog.h \
    src/midi/MidiInput.h \
    src/gui/MidiSettingsDialog.h \
    src/midi/rtmidi/RtError.h \
    src/midi/rtmidi/RtMidi.h \
    src/gui/FileLengthDialog.h \
    src/gui/AboutDialog.h \
    src/gui/DonateDialog.h \
    src/MidiEvent/ChannelPressureEvent.h \
    src/MidiEvent/KeyPressureEvent.h \
    src/gui/EventWidget.h \
    src/midi/SingleNotePlayer.h \
    src/MidiEvent/ControlChangeEvent.h \
    src/MidiEvent/ProgChangeEvent.h \
    src/midi/PlayerThread.h \
    src/midi/MidiPlayer.h \
    src/midi/MidiOutput.h \
    src/tool/StandardTool.h \
    src/gui/ClickButton.h \
    src/tool/SizeChangeTool.h \
    src/tool/EraserTool.h \
    src/tool/NewNoteTool.h \
    src/gui/VelocityWidget.h \
    src/MidiEvent/OnEvent.h \
    src/MidiEvent/OffEvent.h \
    src/gui/ProtocolWidget.h \
    src/gui/ChannelListWidget.h \
    src/gui/EventListWidget.h \
    src/gui/GraphicObject.h \
    src/gui/MainWindow.h \
    src/gui/MatrixWidget.h \
    src/gui/PaintWidget.h \
    src/midi/MidiChannel.h \
    src/midi/MidiFile.h \
    src/tool/EventMoveTool.h \
    src/tool/SelectTool.h \
    src/tool/EventTool.h \
    src/tool/ToolButton.h \
    src/tool/EditorTool.h \
    src/tool/Tool.h \
    src/protocol/ProtocolStep.h \
    src/protocol/ProtocolItem.h \
    src/protocol/ProtocolEntry.h \
    src/protocol/Protocol.h \
    src/MidiEvent/TimeSignatureEvent.h \
    src/MidiEvent/TempoChangeEvent.h \
    src/MidiEvent/UnknownEvent.h \
    src/MidiEvent/NoteOnEvent.h \
    src/MidiEvent/MidiEvent.h
SOURCES += src/midi/SenderThread.cpp \
    src/Terminal.cpp \
    src/gui/RecordDialog.cpp \
    src/midi/MidiInput.cpp \
    src/gui/MidiSettingsDialog.cpp \
    src/midi/rtmidi/RtMidi.cpp \
    src/gui/FileLengthDialog.cpp \
    src/gui/AboutDialog.cpp \
    src/gui/DonateDialog.cpp \
    src/MidiEvent/ChannelPressureEvent.cpp \
    src/MidiEvent/KeyPressureEvent.cpp \
    src/gui/EventWidget.cpp \
    src/midi/SingleNotePlayer.cpp \
    src/MidiEvent/ControlChangeEvent.cpp \
    src/MidiEvent/ProgChangeEvent.cpp \
    src/midi/PlayerThread.cpp \
    src/midi/MidiPlayer.cpp \
    src/midi/MidiOutput.cpp \
    src/tool/StandardTool.cpp \
    src/gui/ClickButton.cpp \
    src/tool/SizeChangeTool.cpp \
    src/tool/EraserTool.cpp \
    src/tool/NewNoteTool.cpp \
    src/gui/VelocityWidget.cpp \
    src/MidiEvent/OnEvent.cpp \
    src/MidiEvent/OffEvent.cpp \
    src/gui/ProtocolWidget.cpp \
    src/gui/ChannelListWidget.cpp \
    src/gui/EventListWidget.cpp \
    src/gui/GraphicObject.cpp \
    src/gui/MainWindow.cpp \
    src/gui/MatrixWidget.cpp \
    src/gui/PaintWidget.cpp \
    src/midi/MidiChannel.cpp \
    src/midi/MidiFile.cpp \
    src/tool/EventMoveTool.cpp \
    src/tool/SelectTool.cpp \
    src/tool/EventTool.cpp \
    src/tool/ToolButton.cpp \
    src/tool/EditorTool.cpp \
    src/tool/Tool.cpp \
    src/protocol/ProtocolStep.cpp \
    src/protocol/ProtocolItem.cpp \
    src/protocol/ProtocolEntry.cpp \
    src/protocol/Protocol.cpp \
    src/MidiEvent/TimeSignatureEvent.cpp \
    src/MidiEvent/TempoChangeEvent.cpp \
    src/MidiEvent/UnknownEvent.cpp \
    src/MidiEvent/NoteOnEvent.cpp \
    src/MidiEvent/MidiEvent.cpp \
    src/main.cpp
FORMS += 
RESOURCES += 
DEFINES += __LINUX_ALSASEQ__
LIBS += -lasound
CONFIG += release
