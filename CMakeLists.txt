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

source_group( "main"        FILES ${main}      ${mainh}      )
source_group( "gui"         FILES ${gui}       ${guih}       )
source_group( "MidiEvent"   FILES ${MidiEvent} ${MidiEventh} )
source_group( "protocol"    FILES ${Protocol}  ${Protocolh}  )
source_group( "midi"        FILES ${Midi}      ${Midih}      )
source_group( "midi/rtmidi" FILES ${RTMidi}    ${RTMidih}    )
source_group( "remote"      FILES ${Remote}    ${Remoteh}    )
source_group( "tool"        FILES ${Tool}      ${Toolh}      )

add_definitions(${Qt5Core_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

add_executable(MidiEditor 
	${main}      ${mainh}      
	${gui}       ${guih}      
	${MidiEvent} ${MidiEventh}
	${Protocol}  ${Protocolh} 
	${Midi}      ${Midih}     
	${RTMidi}    ${RTMidih}   
	${Remote}    ${Remoteh}   
	${Tool}      ${Toolh}     
)
include(ressources.cmake)
if(MSVC)
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml Winmm.lib ${ressources})
else()
	target_link_libraries(MidiEditor Qt5::Widgets Qt5::Network Qt5::Xml )
endif()