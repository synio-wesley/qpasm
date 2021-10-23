cmake_minimum_required(VERSION 3.0)
project(qpasm)

find_package(Qt5 COMPONENTS Core Gui Svg REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    highlighter.cpp
    aboutdialog.cpp
    qtsignalbridge.cpp
    codeedit.cpp
    pasmthread.cpp
    listwidgetitem.cpp
    addmemdialog.cpp
    linevalidator.cpp
    configdialog.cpp
    libpasm/errorcodes.c
    libpasm/helper.c
    libpasm/interpreter.c
    libpasm/memory.c
    libpasm/ui.c
    libpasm/debugger.c
)

set(HEADERS
    mainwindow.h
    highlighter.h
    libpasm/pasm.h
    libpasm/errorcodes.h
    libpasm/opcodes.h
    libpasm/helper.h
    libpasm/interpreter.h
    libpasm/memory.h
    libpasm/ui.h
    aboutdialog.h
    qtsignalbridge.h
    codeedit.h
    pasmthread.h
    libpasm/debugger.h
    listwidgetitem.h
    addmemdialog.h
    linevalidator.h
    configdialog.h
)

set(FORMS
    mainwindow.ui
    aboutdialog.ui
    addmemdialog.ui
    configdialog.ui
)

set(RESOURCES qpasm.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) 

qt5_add_resources(SOURCES ${RESOURCES})

add_executable(qpasm ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

target_link_libraries(qpasm PUBLIC Qt5::Core Qt5::Gui Qt5::Svg)
