cmake_minimum_required(VERSION 2.8.11)

project(MidiEditor)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
	if( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
		set(QT5_DIR "D:/dev/Qt/5.7/msvc2013_64/lib/cmake") 
		SET(ARCH "x64")
	else( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
		set(QT5_DIR "D:/dev/Qt/5.7/msvc2013/lib/cmake") 
		SET(ARCH "x86")
	endif( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
	find_path(Qt5Widgets_DIR Qt5WidgetsConfig.cmake PATHS "${QT5_DIR}/Qt5Widgets" )
	find_path(Qt5Network_DIR Qt5NetworkConfig.cmake PATHS "${QT5_DIR}/Qt5Network" )
	find_path(Qt5Xml_DIR     Qt5XmlConfig.cmake     PATHS "${QT5_DIR}/Qt5Xml"     )
	add_definitions(/W3)
	add_definitions(-D__WINDOWS_MM__ -D_UNICODE)
	MESSAGE("ARCH = ${ARCH}")
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)

# Tell CMake to create the helloworld executable
file( GLOB src src/MidiEvent/KeySignatureEvent.cpp src/remote/RemoteServer.cpp src/gui/InstrumentChooser.cpp 
    src/gui/TransposeDialog.cpp src/MidiEvent/TextEvent.cpp src/gui/TrackListWidget.cpp src/midi/MidiTrack.cpp 
    src/midi/SenderThread.cpp src/Terminal.cpp src/gui/RecordDialog.cpp src/midi/MidiInput.cpp src/midi/rtmidi/RtMidi.cpp 
    src/gui/FileLengthDialog.cpp src/gui/AboutDialog.cpp src/gui/DonateDialog.cpp src/MidiEvent/ChannelPressureEvent.cpp 
    src/MidiEvent/KeyPressureEvent.cpp src/gui/EventWidget.cpp src/midi/SingleNotePlayer.cpp  src/MidiEvent/ControlChangeEvent.cpp src/MidiEvent/ProgChangeEvent.cpp src/midi/PlayerThread.cpp 
    src/midi/MidiPlayer.cpp src/midi/MidiOutput.cpp src/tool/StandardTool.cpp 
    src/gui/ClickButton.cpp src/tool/SizeChangeTool.cpp src/tool/EraserTool.cpp
    src/tool/NewNoteTool.cpp src/MidiEvent/OnEvent.cpp src/MidiEvent/OffEvent.cpp src/gui/ProtocolWidget.cpp
    src/gui/ChannelListWidget.cpp src/gui/EventListWidget.cpp src/gui/GraphicObject.cpp src/gui/MainWindow.cpp
    src/gui/MatrixWidget.cpp src/gui/PaintWidget.cpp src/midi/MidiChannel.cpp src/midi/MidiFile.cpp
    src/tool/EventMoveTool.cpp src/tool/SelectTool.cpp src/tool/EventTool.cpp src/tool/ToolButton.cpp
    src/tool/EditorTool.cpp src/tool/Tool.cpp src/protocol/ProtocolStep.cpp src/protocol/ProtocolItem.cpp src/protocol/ProtocolEntry.cpp src/protocol/Protocol.cpp src/MidiEvent/TimeSignatureEvent.cpp
    src/MidiEvent/TempoChangeEvent.cpp src/MidiEvent/UnknownEvent.cpp src/MidiEvent/NoteOnEvent.cpp src/MidiEvent/MidiEvent.cpp
    src/MidiEvent/PitchBendEvent.cpp src/main.cpp src/gui/MiscWidget.cpp src/MidiEvent/SysExEvent.cpp src/gui/DataEditor.cpp
    src/gui/SettingsWidget.cpp src/gui/SettingsDialog.cpp src/gui/MidiSettingsWidget.cpp
    src/gui/RemoteSettingsWidget.cpp src/midi/Metronome.cpp
    src/gui/NToleQuantizationDialog.cpp src/tool/Selection.cpp
    src/UpdateManager.cpp src/gui/UpdateSettingsWidget.cpp src/gui/UpdateDialog.cpp)
file( GLOB hdr
    src/MidiEvent/KeySignatureEvent.h src/remote/RemoteServer.h src/gui/InstrumentChooser.h
    src/gui/TransposeDialog.h src/MidiEvent/TextEvent.h src/gui/TrackListWidget.h
    src/midi/MidiTrack.h src/midi/SenderThread.h src/Terminal.h src/gui/RecordDialog.h
    src/midi/MidiInput.h src/midi/rtmidi/RtMidi.h src/gui/FileLengthDialog.h src/gui/AboutDialog.h
    src/gui/DonateDialog.h src/MidiEvent/ChannelPressureEvent.h src/MidiEvent/KeyPressureEvent.h
    src/gui/EventWidget.h src/midi/SingleNotePlayer.h src/MidiEvent/ControlChangeEvent.h
    src/MidiEvent/ProgChangeEvent.h src/midi/PlayerThread.h src/midi/MidiPlayer.h
    src/midi/MidiOutput.h src/tool/StandardTool.h src/gui/ClickButton.h src/tool/SizeChangeTool.h
    src/tool/EraserTool.h src/tool/NewNoteTool.h src/MidiEvent/OnEvent.h src/MidiEvent/OffEvent.h
    src/gui/ProtocolWidget.h src/gui/ChannelListWidget.h src/gui/EventListWidget.h src/gui/GraphicObject.h
    src/gui/MainWindow.h src/gui/MatrixWidget.h src/gui/PaintWidget.h src/midi/MidiChannel.h src/midi/MidiFile.h
    src/tool/EventMoveTool.h src/tool/SelectTool.h src/tool/EventTool.h src/tool/ToolButton.h src/tool/EditorTool.h
    src/tool/Tool.h src/protocol/ProtocolStep.h src/protocol/ProtocolItem.h src/protocol/ProtocolEntry.h
    src/protocol/Protocol.h src/MidiEvent/TimeSignatureEvent.h src/MidiEvent/TempoChangeEvent.h
    src/MidiEvent/UnknownEvent.h src/MidiEvent/NoteOnEvent.h src/MidiEvent/PitchBendEvent.h
    src/MidiEvent/MidiEvent.h src/gui/MiscWidget.h src/MidiEvent/SysExEvent.h src/gui/DataEditor.h
    src/gui/SettingsWidget.h src/gui/SettingsDialog.h src/gui/MidiSettingsWidget.h src/gui/RemoteSettingsWidget.h
    src/midi/Metronome.h src/gui/NToleQuantizationDialog.h src/tool/Selection.h src/UpdateManager.h
    src/gui/UpdateSettingsWidget.h src/gui/UpdateDialog.h )
	
add_definitions(${Qt5Core_DEFINITIONS})
	
# Generate code from ui files
#qt5_wrap_ui(UI_HEADERS mainwindow.ui)

# Generate rules for building source files from the resources
#qt5_add_resources(QRCS ressources.qrc)
qt5_add_binary_resources(ressources "${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc" "${CMAKE_CURRENT_SOURCE_DIR}/ressources.qrc")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc" "<!DOCTYPE RCC><RCC version=\"1.0\">
<qresource prefix=\"/\">
    <file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/add.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/align_left.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/align_right.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/all_audible.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/all_invisible.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/all_mute.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/all_visible.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/back.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/back_to_begin.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/change_size.png</file>
    <file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/config.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/connection.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/copy.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/equalize.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/eraser.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/forward.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/load.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/magnet.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/metronome.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/misc_freehand.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/misc_line.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/misc_single.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/move_left_right.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/move_up_down.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/move_up_down_left_right.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/new.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/newnote.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/noicon.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/paste.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/pause.png</file>
    <file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/play.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/quantize.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/record.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/redo.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/refresh.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/save.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/saveas.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/screen_locked.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/screen_unlocked.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/select.png</file>
    <file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/select_box.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/select_left.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/select_right.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/select_single.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/stop.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/stop_record.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/undo.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/zoom_hor_in.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/zoom_hor_out.png</file>	
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/zoom_ver_in.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/tool/zoom_ver_out.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/channelwidget/loud.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/channelwidget/instrument.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/channelwidget/solo.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/channelwidget/visible.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/trackwidget/loud.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/trackwidget/remove.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/trackwidget/rename.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/trackwidget/visible.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/icon.png</file>
	<file>${CMAKE_CURRENT_SOURCE_DIR}/run_environment/graphics/midieditor.png</file>
</qresource>
</RCC>
")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

#qt5_add_resources(RESOURCES example.qrc)

add_executable(MidiEditor ${src} ${hdr})
#qt5_add_binary_resources(MidiEditor "ressources.qrc" "ressources.rcc")
if(MSVC)
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml ${SFML_LIBRARIES} Winmm.lib ${QRCS})
else()
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml )
endif()