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

file( GLOB src 
      src/*.cpp 
	  src/gui/*.cpp
	  src/MidiEvent/*.cpp
	  src/protocol/*.cpp
	  src/midi/*.cpp
	  src/midi/rtmidi/*.cpp
	  src/remote/*.cpp
	  src/tool/*.cpp )
file( GLOB hdr 
      src/*.h 
	  src/gui/*.h
	  src/MidiEvent/*.h
	  src/protocol/*.h
	  src/midi/*.h
	  src/midi/rtmidi/*.h
	  src/remote/*.h
	  src/tool/*.h )

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
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml Winmm.lib ${ressources})
else()
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml )
endif()