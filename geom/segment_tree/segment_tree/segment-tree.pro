TEMPLATE = app 
TARGET = segment_tree

CONFIG += QtGui
QT += opengl

OBJECTS_DIR = bin

INCLUDEPATH += ../visualization/headers 

QMAKE_CXXFLAGS = -std=c++0x -Wall

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++  
}

CONFIG += precompile_header
PRECOMPILED_HEADER = stdafx.h

HEADERS += \
	common.h \
	primitives.h \
	range_tree.h \
	segment_tree.h \
	segment_windowing.h \
	stdafx.h \
	tree.h


SOURCES += \ 
	main.cpp

LIBS += -L../visualization -lvisualization
