cmake_minimum_required(VERSION 2.8.11)

project(MidiEditor)

configure_file(
  ${CMAKE_SOURCE_DIR}/version.cmake.in
  ${CMAKE_BINARY_DIR}/version.cmake
)
include_directories(${CMAKE_BINARY_DIR})

# check compiler
if(MSVC12)
	message(STATUS "Your compiler is managed")
elseif(CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "Your compiler is managed")
else()
	message(FATAL_ERROR "Your compiler is not managed by this CMakeLists. Only MSVC12 or GCC (G++) is managed")
endif()

option(INSTALL_WITH_CPACK "Make deployment on Windows installer or Linux DEB file" OFF )

if(MSVC12)
	option(NO_CONSOLE_MODE "Have a console to debuging" OFF)
	if( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(QT5_DIR "C:/Qt/5.7/msvc2013_64/lib/cmake" CACHE STRING "Additional search paths for QT5")
		SET(ARCH "x64")
		add_definitions(-D__ARCH64__)
	else( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(QT5_DIR "C:/Qt/5.7/msvc2013/lib/cmake" CACHE STRING "Additional search paths for QT5")
		SET(ARCH "x86")
	endif( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
	find_path(Qt5Widgets_DIR Qt5WidgetsConfig.cmake PATHS "${QT5_DIR}/Qt5Widgets" )
	find_path(Qt5Network_DIR Qt5NetworkConfig.cmake PATHS "${QT5_DIR}/Qt5Network" )
	find_path(Qt5Xml_DIR     Qt5XmlConfig.cmake     PATHS "${QT5_DIR}/Qt5Xml"     )
	add_definitions(/W3)
	add_definitions(-D__WINDOWS_MM__ -D_UNICODE)
	if(NO_CONSOLE_MODE)
		add_definitions(-DNO_CONSOLE_MODE)
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # multi processor compiliation.
	MESSAGE(STATUS "ARCH = ${ARCH}")
else()
    add_definitions(-std=c++11)
	add_definitions(-D__LINUX_ALSASEQ__)
	add_definitions(-D__LINUX_ALSA__)
	set(QT5_DIR "~/Qt/5.7/gcc_64/lib/cmake" CACHE STRING "Additional search paths for QT5")
	find_path(Qt5Widgets_DIR Qt5WidgetsConfig.cmake PATHS "${QT5_DIR}/Qt5Widgets" )
	find_path(Qt5Network_DIR Qt5NetworkConfig.cmake PATHS "${QT5_DIR}/Qt5Network" )
	find_path(Qt5Xml_DIR     Qt5XmlConfig.cmake     PATHS "${QT5_DIR}/Qt5Xml"     )
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml     REQUIRED)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)
#add_definitions(${Qt5Core_DEFINITIONS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

file( GLOB main      src/*.cpp            )
file( GLOB gui       src/gui/*.cpp        )
file( GLOB MidiEvent src/MidiEvent/*.cpp  )
file( GLOB Protocol  src/protocol/*.cpp   )
file( GLOB Midi      src/midi/*.cpp       )
file( GLOB RTMidi    src/midi/rtmidi/*.cpp)
file( GLOB Remote    src/remote/*.cpp     )
file( GLOB Tool      src/tool/*.cpp       )

file( GLOB mainh      src/*.h            )
file( GLOB guih       src/gui/*.h        )
file( GLOB MidiEventh src/MidiEvent/*.h  )
file( GLOB Protocolh  src/protocol/*.h   )
file( GLOB Midih      src/midi/*.h       )
file( GLOB RTMidih    src/midi/rtmidi/*.h)
file( GLOB Remoteh    src/remote/*.h     )
file( GLOB Toolh      src/tool/*.h       )

source_group( "gui"            FILES ${gui}       )
source_group( "gui\\inc"       FILES ${guih}      )
source_group( "MidiEvent"      FILES ${MidiEvent} )
source_group( "MidiEvent\\inc" FILES ${MidiEventh})
source_group( "protocol"       FILES ${Protocol}  )
source_group( "protocol\\inc"  FILES ${Protocolh} )
source_group( "midi"           FILES ${Midi}      )
source_group( "midi\\inc"      FILES ${Midih}     )
source_group( "remote"         FILES ${Remote}    )
source_group( "remote\\inc"    FILES ${Remoteh}   )
source_group( "tool"           FILES ${Tool}      )
source_group( "tool\\inc"      FILES ${Toolh}     )
source_group( ""               FILES ${main} ${mainh} )
source_group( "midi\\rtmidi"   FILES ${RTMidi} ${RTMidih} )

add_executable(MidiEditor
               ${main}      ${mainh}
               ${RTMidi}    ${RTMidih}
               ${Protocol}  ${Protocolh}
               ${Midi}      ${Midih}
               ${gui}       ${guih}
               ${MidiEvent} ${MidiEventh}
               ${Remote}    ${Remoteh}
               ${Tool}      ${Toolh} midieditor.rc)

include(ressources.cmake)
set(MidiEditorDep Qt5::Widgets Qt5::Network Qt5::Xml)
if(MSVC12)
	target_link_libraries(MidiEditor Winmm.lib ${MidiEditorDep})
	if(NO_CONSOLE_MODE)
		set_target_properties(MidiEditor PROPERTIES  LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	endif()
    set(QT5_DLLs_DEP_RELEASE "${QT5_DIR}/../../bin/Qt5Widgets.dll"
                             "${QT5_DIR}/../../bin/Qt5Network.dll"
                             "${QT5_DIR}/../../bin/Qt5Xml.dll"
                             "${QT5_DIR}/../../bin/Qt5Gui.dll"
                             "${QT5_DIR}/../../bin/Qt5Core.dll")
    set(QT5_DLLs_DEP_DEBUG   "${QT5_DIR}/../../bin/Qt5Widgetsd.dll"
                             "${QT5_DIR}/../../bin/Qt5Networkd.dll"
                             "${QT5_DIR}/../../bin/Qt5Xmld.dll"
                             "${QT5_DIR}/../../bin/Qt5Guid.dll"
                             "${QT5_DIR}/../../bin/Qt5Cored.dll")
    file(COPY ${QT5_DLLs_DEP_RELEASE} ${QT5_DLLs_DEP_DEBUG} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
	target_link_libraries(MidiEditor asound sfml-system sfml-audio pthread ${MidiEditorDep})
endif()
add_custom_command(TARGET MidiEditor PRE_BUILD COMMAND cmake -P ${CMAKE_BINARY_DIR}/version.cmake)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/midieditor.ico" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/metronome)

if(INSTALL_WITH_CPACK)
	install(TARGETS MidiEditor DESTINATION bin)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ressources.rcc" DESTINATION bin)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav" DESTINATION bin/metronome)
	if(WIN32)#Windows
		install(FILES ${QT5_DLLs_DEP_RELEASE} DESTINATION bin)
		set(CPACK_GENERATOR NSIS)
		set(CPACK_PACKAGE_INSTALL_DIRECTORY "MidiEditor")
		set(CPACK_PACKAGE_EXECUTABLES "MidiEditor" "MidiEditor" ${CPACK_PACKAGE_EXECUTABLES}) 
		set(CPACK_NSIS_MODIFY_PATH OFF)
	else() #UNIX
		set(CPACK_GENERATOR "DEB")
		SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Amaury Bréhéret") #required
	endif()
	set(CPACK_PACKAGE_NAME "MidiEditor")
	set(CPACK_PACKAGE_VENDOR "https://github.com/abreheret/MidiEditor")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MidiEditor - Installation ")
	set(CPACK_PACKAGE_VERSION "3.1.1")
	set(CPACK_PACKAGE_VERSION_MAJOR "3")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	set(CPACK_PACKAGE_VERSION_PATCH "1")
	INCLUDE(CPack)
endif()
