cmake_minimum_required(VERSION 2.8.11)

project(MidiEditor)

message(STATUS "Running CMake for project MidiEditor...")
message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}")

configure_file(
  ${CMAKE_SOURCE_DIR}/version.cmake.in
  ${CMAKE_BINARY_DIR}/version.cmake
)
include_directories(${CMAKE_BINARY_DIR})

option(INSTALL_WITH_CPACK "Make deployment on Windows installer or Linux DEB file" OFF)

message(STATUS "Checking your compiler...")

if (WIN32)
	if (MSVC14) # Microsoft Visual Studio 2015
		message(STATUS "Your compiler, MSVC 2015 ${CMAKE_CXX_COMPILER_VERSION}, is supported.")
		set(COMPILER_CODE "msvc2015")
	elseif (MSVC12) # Microsoft Visual Studio 2013
		message(STATUS "Your compiler, MSVC 2013 ${CMAKE_CXX_COMPILER_VERSION}, is supported.")
		set(COMPILER_CODE "msvc2013")
	elseif (MSVC15) # Microsoft Visual Studio 2017
		message(STATUS "Your compiler, MSVC 2017 ${CMAKE_CXX_COMPILER_VERSION}, is supported.")
		set(COMPILER_CODE, "msvc2017")
	elseif (MINGW) # MinGW
		message(STATUS "Your compiler, MinGW ${CMAKE_CXX_COMPILER_VERSION} is supported.")
		set(COMPILER_CODE, "mingw53_32")
	endif()
	option(NO_CONSOLE_MODE "Start MidiEditor in a console window for debugging" OFF)
	if(NO_CONSOLE_MODE)
		add_definitions(-DNO_CONSOLE_MODE)
	endif()

	    # MinGW isn't currently supported on 64-bit.
	if( NOT MINGW AND CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(QT5_DIR_GUESS "C:/Qt/5.9.1/${COMPILER_CODE}_64/lib/cmake" CACHE STRING "Additional search paths for QT5")
		SET(ARCH "x64")

	elseif( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(QT5_DIR_GUESS "C:/Qt/5.9.1/${COMPILER_CODE}/lib/cmake" CACHE STRING "Additional search paths for Qt 5")
		SET(ARCH "x86")

	endif( CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8 )
	add_definitions(/W3)
	add_definitions(-D__WINDOWS_MM__ -D_UNICODE)
	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	endif()

	MESSAGE(STATUS "ARCH = ${ARCH}")

elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND APPLE) # Apple Clang

message(STATUS "Your compiler, AppleClang ${CMAKE_CXX_COMPILER_VERSION}, is supported.")
    add_definitions(-std=c++11)

	set(CMAKE_MACOSX_RPATH 1)

	option(USE_JACK "Use Jack Audio as well as CoreAudio for IO" OFF)
	if (USE_JACK)
		message(STATUS "Enabling Jack Audio support...")
		add_definitions(-D__UNIX_JACK__)
	endif()
	add_definitions(-D__MACOSX_CORE__)
	set(QT5_DIR_GUESS "~/Qt/5.9.1/clang_64/lib/cmake" CACHE STRING "Additional search paths for Qt 5" )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

elseif (CMAKE_COMPILER_IS_GNUCXX) # Gnu C++

	message(STATUS "Your compiler, ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, is supported.")
	add_definitions(-std=c++11)
	option(USE_JACK "Use Jack Audio as well as ALSA for IO" OFF)
	if (USE_JACK)
		message(STATUS "Enabling Jack Audio support...")
		add_definitions(-D__UNIX_JACK__)
	endif()
	add_definitions(-D__LINUX_ALSASEQ__)
	add_definitions(-D__LINUX_ALSA__)
	set(QT5_DIR_GUESS "~/Qt/5.9.1/gcc_64/lib/cmake" CACHE STRING "Additional search paths for QT5")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()

	message(FATAL_ERROR "Your compiler is not supported by this CMakeLists. \
Please use Visual Studio 2015 (2013 or 2017 also work), MinGW, AppleClang (Xcode), or GCC (G++) instead.")
endif()

if(ARCH STREQUAL "x64" )
	add_definitions(-D__ARCH64__)
endif()

# Avoid Unicode -> QString issues.
add_definitions(-DQT_NO_CAST_TO_ASCII)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#add_definitions(${Qt5Core_DEFINITIONS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

message(STATUS "Checking Qt version...")

find_package(Qt5 COMPONENTS Core Widgets Multimedia Xml Network REQUIRED PATHS ${QT5_DIR_GUESS})
# Check Qt version
if (Qt5Core_FOUND)
	if (Qt5Core_VERSION VERSION_LESS 5.9)
		        message(FATAL_ERROR "Your Qt version (${Qt5Core_VERSION}) is out of date. \
Please update to Qt >= 5.9.")
    else()
		message(STATUS "Found Qt version ${Qt5Core_VERSION}.")
	endif()
else()
	message(FATAL_ERROR "You don't have Qt 5 installed. \
Alternatively, you may need to set CMAKE_PREFIX_PATH to
        /path/to/Qt/<Qt Version>/<Compiler>/lib/cmake
and try again. \
On Windows, this is usually in C:/Qt, and on Linux/Mac, it is usually ~/Qt.")
endif()
find_package(Qt5Widgets    REQUIRED HINTS ${QT5_DIR_HINT})
find_package(Qt5Network    REQUIRED HINTS ${QT5_DIR_HINT})
find_package(Qt5Xml        REQUIRED HINTS ${QT5_DIR_HINT})
find_package(Qt5Multimedia REQUIRED HINTS ${QT5_DIR_HINT})
if (APPLE)
	find_package(Qt5MacExtras  REQUIRED HINTS ${QT5_DIR_HINT})
endif()

message(STATUS, "Collecting files…")
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

include(ressources.cmake)

if (APPLE)
	set(UI_TYPE MACOSX_BUNDLE)

	set(MACOSX_BUNDLE_INFO_STRING "MidiEditor")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.abreheret.MidiEditor")
	set(MACOSX_BUNDLE_BUNDLE_NAME "MidiEditor")
	set(MACOSX_BUNDLE_ICON_FILE "MidiEditor.icns")

	add_executable(MidiEditor
		    MACOSX_BUNDLE
			  ${main}      ${mainh}
		   ${RTMidi}    ${RTMidih}
		   ${Protocol}  ${Protocolh}
		   ${Midi}      ${Midih}
		   ${gui}       ${guih}
		   ${MidiEvent} ${MidiEventh}
		   ${Remote}    ${Remoteh}
		   ${Tool}      ${Toolh} midieditor.rc
		   run_environment/metronome/metronome-01.wav
		   run_environment/MidiEditor.icns
		   ${CMAKE_BINARY_DIR}/ressources.rcc)

	set_target_properties(MidiEditor PROPERTIES RESOURCE ${CMAKE_BINARY_DIR}/ressources.rcc)
	set_source_files_properties(run_environment/MidiEditor.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	set(MidiEditorDep Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Multimedia Qt5::MacExtras)
else()
	add_executable(MidiEditor
		   ${main}      ${mainh}
		   ${RTMidi}    ${RTMidih}
		   ${Protocol}  ${Protocolh}
		   ${Midi}      ${Midih}
		   ${gui}       ${guih}
		   ${MidiEvent} ${MidiEventh}
		   ${Remote}    ${Remoteh}
		   ${Tool}      ${Toolh} midieditor.rc)
	set(MidiEditorDep Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Multimedia)

endif()



message(STATUS "Linking libraries...")
if(WIN32)
	target_link_libraries(MidiEditor Winmm.lib ${MidiEditorDep})
	if(NO_CONSOLE_MODE)
		if (MINGW)
			set_target_properties(MidiEditor PROPERTIES LINK_FLAGS "-mwindows")
		else()
			set_target_properties(MidiEditor PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
		endif()
	endif()
	set(QT5_DLLs_DEP_RELEASE "${QT5_DIR}/../../bin/Qt5Widgets.dll"
		                     "${QT5_DIR}/../../bin/Qt5Network.dll"
							 "${QT5_DIR}/../../bin/Qt5Xml.dll"
							 "${QT5_DIR}/../../bin/Qt5Gui.dll"
							 "${QT5_DIR}/../../bin/Qt5Core.dll"
							 "${QT5_DIR}/../../bin/Qt5Multimedia.dll")
						set(QT5_DLLs_DEP_DEBUG   "${QT5_DIR}/../../bin/Qt5Widgetsd.dll"
							 "${QT5_DIR}/../../bin/Qt5Networkd.dll"
							 "${QT5_DIR}/../../bin/Qt5Xmld.dll"
							 "${QT5_DIR}/../../bin/Qt5Guid.dll"
							 "${QT5_DIR}/../../bin/Qt5Cored.dll"
							 "${QT5_DIR}/../../bin/Qt5Multimediad.dll")
						file(COPY ${QT5_DLLs_DEP_RELEASE} ${QT5_DLLs_DEP_DEBUG} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

elseif(APPLE)
	message(STATUS "Setting up macOS frameworks...")
	include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks )
	find_library(CORE_FOUNDATION CoreFoundation)
	find_library(CORE_MIDI CoreMIDI )
	find_library(CORE_AUDIO CoreAudio )
	mark_as_advanced (CORE_FOUNDATION
		          CORE_MIDI
			  CORE_AUDIO)
	set(EXTRA_LIBS ${CORE_FOUNDATION} ${CORE_AUDIO} ${CORE_MIDI} )
	if (USE_JACK)
		include_directories(/usr/local/lib)
		link_directories(/usr/local/lib)
		target_link_libraries(MidiEditor ${EXTRA_LIBS} jack ${MidiEditorDep})
	else()
		target_link_libraries(MidiEditor ${EXTRA_LIBS} ${MidiEditorDep})
	endif()
else()
	target_link_libraries(MidiEditor asound sfml-system sfml-audio pthread ${MidiEditorDep})
endif()

if (MSVC)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/GR-")
else()
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fno-rtti")
endif()

#add_custom_command(TARGET MidiEditor PRE_BUILD COMMAND cmake -P ${CMAKE_BINARY_DIR}/version.cmake)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/midieditor.ico" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/metronome)

if(INSTALL_WITH_CPACK)
	message(STATUS "Configuring CPack...")
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
		SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Amaury Bréhéret")
		if (USE_JACK)
			set (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5:amd64 (>=5.9.1), libqt5sound5:amd64 (>=5.9.1), libqt5network5:amd64 (>=5.9.1), libqt5xml:amd64 (>=5.9.1), libsfml-system,libsfml-audio,libasound,libpthread,jack2")
		else()
			set (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5:amd64 (>=5.9.1), libqt5sound5:amd64 (>=5.9.1), libqt5network5:amd64 (>=5.9.1), libqt5xml:amd64 (>=5.9.1), libsfml-system,libsfml-audio,libasound,libpthread")
		endif()
		#set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default (>= 5.9.1),libsfml-system,libsfml-audio,libasound,libpthread")
		#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libQt5Widgets,libQt5Sound,libQt5Network,libQt5Xml,libQt5Core,libsfml-system,libsfml-audio,libasound,libpthread")
		set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
	endif()
	set(CPACK_PACKAGE_CONTACT "Amaury Bréhéret abreheret@gmail.com")
	set(CPACK_PACKAGE_NAME "MidiEditor")
	set(CPACK_PACKAGE_VENDOR "https://github.com/abreheret/MidiEditor")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MidiEditor - Installation ")
	set(CPACK_PACKAGE_VERSION "3.1.1")
	set(CPACK_PACKAGE_VERSION_MAJOR "3")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	set(CPACK_PACKAGE_VERSION_PATCH "1")
	include(CPack)
endif()
