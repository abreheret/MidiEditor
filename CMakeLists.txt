cmake_minimum_required(VERSION 2.8.11)

project(MidiEditor)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)

if(WIN32)
	if( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
	    set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013_64/lib/cmake/Qt5Widgets" ${CMAKE_MODULE_PATH})
		set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013_64/lib/cmake/Qt5Network" ${CMAKE_MODULE_PATH})
		set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013_64/lib/cmake/Qt5Xml"     ${CMAKE_MODULE_PATH})
		#find_path(Qt5Widgets_DIR Qt5WidgetsConfig.cmake PATHS "D:/DEV/Qt/5.7/msvc2013_64/lib/cmake/Qt5Widgets" )
		#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sfml-2.3.2/vc10x64/cmake/Modules" ${CMAKE_MODULE_PATH})
		#set(SFML_ROOT "${CMAKE_SOURCE_DIR}/sfml-2.3.2/vc10x64/")
		SET(ARCH "x64")
	else( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
	    set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013/lib/cmake/Qt5Widgets" ${CMAKE_MODULE_PATH})
		set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013/lib/cmake/Qt5Network" ${CMAKE_MODULE_PATH})
		set(CMAKE_MODULE_PATH "D:/DEV/Qt/5.7/msvc2013/lib/cmake/Qt5Xml"     ${CMAKE_MODULE_PATH})
		#find_path(Qt5Widgets_DIR Qt5WidgetsConfig.cmake PATHS "D:/DEV/Qt/5.7/msvc2013/lib/cmake/Qt5Widgets" )
		#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sfml-2.3.2/vc10x86/cmake/Modules" ${CMAKE_MODULE_PATH})
		#set(SFML_ROOT "${CMAKE_SOURCE_DIR}/sfml-2.3.2/vc10x86/")
		SET(ARCH "x86")
	endif( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
	
	add_definitions(/W3)
	add_definitions(-D__WINDOWS_MM__ -D_UNICODE)
	#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/D_UNICODE")
	#find_package(SFML COMPONENTS audio REQUIRED)
	include_directories(${SFML_INCLUDE_DIR})
	MESSAGE("ARCH = ${ARCH}")
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)

# Generate code from ui files
#qt5_wrap_ui(UI_HEADERS mainwindow.ui)

# Generate rules for building source files from the resources
#qt5_add_resources(QRCS resources.qrc)

# Tell CMake to create the helloworld executable
file( GLOB src 
    src/MidiEvent/KeySignatureEvent.cpp 
    src/remote/RemoteServer.cpp 
    src/gui/InstrumentChooser.cpp 
    src/gui/TransposeDialog.cpp 
    src/MidiEvent/TextEvent.cpp 
    src/gui/TrackListWidget.cpp 
    src/midi/MidiTrack.cpp 
    src/midi/SenderThread.cpp 
    src/Terminal.cpp 
    src/gui/RecordDialog.cpp 
    src/midi/MidiInput.cpp 
    src/midi/rtmidi/RtMidi.cpp 
    src/gui/FileLengthDialog.cpp 
    src/gui/AboutDialog.cpp 
    src/gui/DonateDialog.cpp 
    src/MidiEvent/ChannelPressureEvent.cpp 
    src/MidiEvent/KeyPressureEvent.cpp 
    src/gui/EventWidget.cpp 
    src/midi/SingleNotePlayer.cpp 
    src/MidiEvent/ControlChangeEvent.cpp 
    src/MidiEvent/ProgChangeEvent.cpp 
    src/midi/PlayerThread.cpp 
    src/midi/MidiPlayer.cpp 
    src/midi/MidiOutput.cpp 
    src/tool/StandardTool.cpp 
    src/gui/ClickButton.cpp
    src/tool/SizeChangeTool.cpp
    src/tool/EraserTool.cpp
    src/tool/NewNoteTool.cpp
    src/MidiEvent/OnEvent.cpp
    src/MidiEvent/OffEvent.cpp
    src/gui/ProtocolWidget.cpp
    src/gui/ChannelListWidget.cpp
    src/gui/EventListWidget.cpp
    src/gui/GraphicObject.cpp
    src/gui/MainWindow.cpp
    src/gui/MatrixWidget.cpp
    src/gui/PaintWidget.cpp
    src/midi/MidiChannel.cpp
    src/midi/MidiFile.cpp
    src/tool/EventMoveTool.cpp
    src/tool/SelectTool.cpp
    src/tool/EventTool.cpp
    src/tool/ToolButton.cpp
    src/tool/EditorTool.cpp
    src/tool/Tool.cpp
    src/protocol/ProtocolStep.cpp
    src/protocol/ProtocolItem.cpp
    src/protocol/ProtocolEntry.cpp
    src/protocol/Protocol.cpp
    src/MidiEvent/TimeSignatureEvent.cpp
    src/MidiEvent/TempoChangeEvent.cpp
    src/MidiEvent/UnknownEvent.cpp
    src/MidiEvent/NoteOnEvent.cpp
    src/MidiEvent/MidiEvent.cpp
    src/MidiEvent/PitchBendEvent.cpp
    src/main.cpp
    src/gui/MiscWidget.cpp
    src/MidiEvent/SysExEvent.cpp
    src/gui/DataEditor.cpp
    src/gui/SettingsWidget.cpp
    src/gui/SettingsDialog.cpp
    src/gui/MidiSettingsWidget.cpp
    src/gui/RemoteSettingsWidget.cpp
    src/midi/Metronome.cpp
    src/gui/NToleQuantizationDialog.cpp
    src/tool/Selection.cpp
    src/UpdateManager.cpp
    src/gui/UpdateSettingsWidget.cpp
    src/gui/UpdateDialog.cpp)
file( GLOB hdr
    src/MidiEvent/KeySignatureEvent.h
    src/remote/RemoteServer.h
    src/gui/InstrumentChooser.h
    src/gui/TransposeDialog.h
    src/MidiEvent/TextEvent.h
    src/gui/TrackListWidget.h
    src/midi/MidiTrack.h
    src/midi/SenderThread.h
    src/Terminal.h
    src/gui/RecordDialog.h
    src/midi/MidiInput.h
    src/midi/rtmidi/RtMidi.h
    src/gui/FileLengthDialog.h
    src/gui/AboutDialog.h
    src/gui/DonateDialog.h
    src/MidiEvent/ChannelPressureEvent.h
    src/MidiEvent/KeyPressureEvent.h
    src/gui/EventWidget.h
    src/midi/SingleNotePlayer.h
    src/MidiEvent/ControlChangeEvent.h
    src/MidiEvent/ProgChangeEvent.h
    src/midi/PlayerThread.h
    src/midi/MidiPlayer.h
    src/midi/MidiOutput.h
    src/tool/StandardTool.h
    src/gui/ClickButton.h
    src/tool/SizeChangeTool.h
    src/tool/EraserTool.h
    src/tool/NewNoteTool.h
    src/MidiEvent/OnEvent.h
    src/MidiEvent/OffEvent.h
    src/gui/ProtocolWidget.h
    src/gui/ChannelListWidget.h
    src/gui/EventListWidget.h
    src/gui/GraphicObject.h
    src/gui/MainWindow.h
    src/gui/MatrixWidget.h
    src/gui/PaintWidget.h
    src/midi/MidiChannel.h
    src/midi/MidiFile.h
    src/tool/EventMoveTool.h
    src/tool/SelectTool.h
    src/tool/EventTool.h
    src/tool/ToolButton.h
    src/tool/EditorTool.h
    src/tool/Tool.h
    src/protocol/ProtocolStep.h
    src/protocol/ProtocolItem.h
    src/protocol/ProtocolEntry.h
    src/protocol/Protocol.h
    src/MidiEvent/TimeSignatureEvent.h
    src/MidiEvent/TempoChangeEvent.h
    src/MidiEvent/UnknownEvent.h
    src/MidiEvent/NoteOnEvent.h
    src/MidiEvent/PitchBendEvent.h
    src/MidiEvent/MidiEvent.h
    src/gui/MiscWidget.h
    src/MidiEvent/SysExEvent.h
    src/gui/DataEditor.h
    src/gui/SettingsWidget.h
    src/gui/SettingsDialog.h
    src/gui/MidiSettingsWidget.h
    src/gui/RemoteSettingsWidget.h
    src/midi/Metronome.h
    src/gui/NToleQuantizationDialog.h
    src/tool/Selection.h
    src/UpdateManager.h
    src/gui/UpdateSettingsWidget.h
    src/gui/UpdateDialog.h )
	
add_executable(MidiEditor ${src} ${hdr})
if(MSVC)
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml ${SFML_LIBRARIES} Winmm.lib)
else()
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml )
endif()