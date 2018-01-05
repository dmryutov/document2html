#-------------------------------------------------
#
# Project created by QtCreator 2017-01-22T21:34:35
#
#-------------------------------------------------

TARGET   = "document2html"
TEMPLATE = app
LANGUAGE = C++

CONFIG  += c++11
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  -= qt


macx {
	INCLUDEPATH += $${PWD}/libs/tidy/include_next
	LIBS += -lcurl \
			-liconv \
			$${PWD}/libs/tidy/lib/libtidy.5.5.75.dylib
			#-ltidy
}
win32|win64 {
	INCLUDEPATH += $${PWD}/libs/curl/include \
				   $${PWD}/libs/iconv/include \
				   $${PWD}/libs/pymagic/include \
				   $${PWD}/libs/tidy/include

	HEADERS += libs/dirent.h

	LIBS += -L$${PWD}/libs/curl/lib/ -llibcurl \
			-L$${PWD}/libs/iconv/lib/ -llibiconvStatic \
			-L$${PWD}/libs/pymagic/lib/ -llibmagic \
			-L$${PWD}/libs/tidy/lib/ -ltidys
}
unix:!macx {
	LIBS += -ltidy \
			-lcurl
}

SOURCES += main.cpp \
		   libs/tools.cpp \
		   libs/curlwrapper/curlwrapper.cpp \
		   libs/fileext/archive/archive.cpp \
		   libs/fileext/cfb/cfb.cpp \
		   libs/fileext/ooxml/ooxml.cpp \
		   libs/fileext/csv/csv.cpp \
		   libs/fileext/docx/docx.cpp \
		   libs/fileext/excel/book.cpp \
		   libs/fileext/excel/excel.cpp \
		   libs/fileext/excel/format.cpp \
		   libs/fileext/excel/formula.cpp \
		   libs/fileext/excel/sheet.cpp \
		   libs/fileext/excel/xlsx.cpp \
		   libs/fileext/html/html.cpp \
		   libs/fileext/rtf/formatting.cpp \
		   libs/fileext/rtf/keyword.cpp \
		   libs/fileext/rtf/rtf.cpp \
		   libs/fileext/rtf/table.cpp \
		   libs/fileext/txt/txt.cpp \
		   libs/fileext/xml/xml.cpp \
		   libs/fileext/doc/doc.cpp \
		   libs/fileext/epub/epub.cpp \
		   libs/fileext/json/json.cpp \
		   libs/fileext/odt/odt.cpp \
		   libs/fileext/pdf/pdf.cpp \
		   libs/fileext/ppt/ppt.cpp \
		   libs/fileext/fileext.cpp \
		   libs/getoptpp/getoptpp.cpp \
		   libs/miniz/miniz.c \
		   libs/pugixml/pugixml.cpp \
		   libs/pymagic/pymagic.cpp \
		   libs/encoding/encoding.cpp \
		   libs/lodepng/lodepng.cpp

HEADERS += libs/tools.hpp \
		   libs/curlwrapper/curlwrapper.hpp \
		   libs/fileext/archive/archive.hpp \
		   libs/fileext/cfb/cfb.hpp \
		   libs/fileext/ooxml/ooxml.hpp \
		   libs/fileext/csv/csv.hpp \
		   libs/fileext/docx/docx.hpp \
		   libs/fileext/excel/biffh.hpp \
		   libs/fileext/excel/book.hpp \
		   libs/fileext/excel/excel.hpp \
		   libs/fileext/excel/format.hpp \
		   libs/fileext/excel/frmt.hpp \
		   libs/fileext/excel/formula.hpp \
		   libs/fileext/excel/sheet.hpp \
		   libs/fileext/excel/xlsx.hpp \
		   libs/fileext/html/html.hpp \
		   libs/fileext/rtf/formatting.hpp \
		   libs/fileext/rtf/keyword.hpp \
		   libs/fileext/rtf/rtf.hpp \
		   libs/fileext/rtf/table.hpp \
		   libs/fileext/txt/txt.hpp \
		   libs/fileext/xml/xml.hpp \
		   libs/fileext/doc/doc.hpp \
		   libs/fileext/epub/epub.hpp \
		   libs/fileext/json/json.hpp \
		   libs/fileext/odt/odt.hpp \
		   libs/fileext/pdf/pdf.hpp \
		   libs/fileext/ppt/ppt.hpp \
		   libs/fileext/fileext.hpp \
		   libs/getoptpp/getoptpp.hpp \
		   libs/miniz/miniz.c \
		   libs/pugixml/pugiconfig.hpp \
		   libs/pugixml/pugixml.hpp \
		   libs/pymagic/pymagic.hpp \
		   libs/json.hpp \
		   libs/encoding/encoding.hpp \
		   libs/lodepng/lodepng.h
