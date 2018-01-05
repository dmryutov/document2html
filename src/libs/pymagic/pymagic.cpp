/**
 * @brief     Interface to the libmagic file type identification library
 * @package   pymagic
 * @file      pymagic.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright ahupp (https://github.com/ahupp/python-magic)
 * @date      10.08.2016 -- 29.10.2017
 */
#include <memory>
#include <unordered_map>

#include "../tools.hpp"

#include "pymagic.hpp"
#include <iostream>

namespace pymagic {

/** MIME extension list */
const std::unordered_map<std::string, std::string> MIME_EXTENSION_LIST {
	{"application/x-shar",              "shar"},
	{"text/vnd.wap.wmlscript",          "wmls"},
	{"application/postscript",          "ai"},
	{"video/quicktime",                 "mov"},
	{"application/x-dvi",               "dvi"},
	{"image/x-xpixmap",                 "xpm"},
	{"application/mathml+xml",          "mathml"},
	{"application/srgs+xml",            "grxml"},
	{"application/x-director",          "dcr"},
	{"text/richtext",                   "rtx"},
	{"image/png",                       "png"},
	{"image/x-ms-bmp",                  "bmp"},
	{"chemical/x-pdb",                  "pdb"},
	{"audio/x-pn-realaudio",            "ram"},
	{"audio/x-wav",                     "wav"},
	{"video/x-flv",                     "flv"},
	{"application/oda",                 "oda"},
	{"model/iges",                      "igs"},
	{"application/x-netcdf",            "nc"},
	{"application/epub+zip",            "epub"},
	{"application/x-stuffit",           "sit"},
	{"text/html",                       "html"},
	{"application/smil",                "smi"},
	{"text/x-setext",                   "etx"},
	{"x-conference/x-cooltalk",         "ice"},
	{"model/mesh",                      "msh"},
	{"application/x-tcl",               "tcl"},
	{"text/calendar",                   "ics"},
	{"text/sgml",                       "sgml"},
	{"application/x-cpio",              "cpio"},
	{"application/ogg",                 "ogg"},
	{"video/vnd.mpegurl",               "m4u"},
	{"application/vnd.mif",             "mif"},
	{"application/x-latex",             "latex"},
	{"image/ief",                       "ief"},
	{"application/x-ustar",             "ustar"},
	{"application/mac-compactpro",      "cpt"},
	{"application/x-chess-pgn",         "pgn"},
	{"image/x-rgb",                     "rgb"},
	{"application/xslt+xml",            "xslt"},
	{"audio/mpeg",                      "mp3"},
	{"application/vnd.wap.wmlc",        "wmlc"},
	{"application/xml",                 "xml"},
	{"application/x-cdlink",            "vcd"},
	{"image/x-portable-graymap",        "pgm"},
	{"application/x-gtar",              "gtar"},
	{"application/octet-stream",        "bin"},
	{"application/vnd.mozilla.xul+xml", "xul"},
	{"text/plain",                      "txt"},
	{"application/x-wais-source",       "src"},
	{"application/atom+xml",            "atom"},
	{"application/x-troff-man",         "man"},
	{"application/x-tex",               "tex"},
	{"image/x-xwindowdump",             "xwd"},
	{"application/x-bcpio",             "bcpio"},
	{"text/css",                        "css"},
	{"application/zip",                 "zip"},
	{"application/x-rar",               "rar"},
	{"application/x-7z-compressed",     "7z"},
	{"application/x-bzip2",             "bz2"},
	{"application/x-gzip",              "gz"},
	{"image/svg+xml",                   "svg"},
	{"model/vrml",                      "wrl"},
	{"application/x-sh",                "sh"},
	{"application/vnd.wap.wmlscriptc",  "wmlsc"},
	{"application/pdf",                 "pdf"},
	{"application/x-troff-me",          "me"},
	{"application/x-troff-ms",          "ms"},
	{"image/gif",                       "gif"},
	{"image/tiff",                      "tiff"},
	{"application/andrew-inset",        "ez"},
	{"application/srgs",                "gram"},
	{"image/x-portable-anymap",         "pnm"},
	{"text/vnd.wap.wml",                "wml"},
	{"application/x-javascript",        "js"},
	{"application/json",                "json"},
	{"application/javascript",          "jsonp"},
	{"application/x-sv4crc",            "sv4crc"},
	{"application/xhtml+xml",           "xhtml"},
	{"image/vnd.djvu",                  "djvu"},
	{"image/x-icon",                    "ico"},
	{"video/mpeg",                      "mpg"},
	{"audio/x-aiff",                    "aif"},
	{"application/x-csh",               "csh"},
	{"application/mac-binhex40",        "hqx"},
	{"image/x-cmu-raster",              "ras"},
	{"image/jpeg",                      "jpg"},
	{"application/vnd.wap.wbxml",       "wbxml"},
	{"application/x-futuresplash",      "spl"},
	{"application/x-texinfo",           "texinfo"},
	{"application/voicexml+xml",        "vxml"},
	{"image/vnd.wap.wbmp",              "wbmp"},
	{"image/x-portable-bitmap",         "pbm"},
	{"application/xml-dtd",             "dtd"},
	{"audio/x-mpegurl",                 "m3u"},
	{"chemical/x-xyz",                  "xyz"},
	{"application/vnd.ms-excel",        "xls"},
	{"image/bmp",                       "bmp"},
	{"image/cgm",                       "cgm"},
	{"video/x-sgi-movie",               "movie"},
	{"application/x-hdf",               "hdf"},
	{"audio/midi",                      "mid"},
	{"image/x-portable-pixmap",         "ppm"},
	{"application/x-koan",              "skp"},
	{"application/rdf+xml",             "rdf"},
	{"image/x-xbitmap",                 "xbm"},
	{"application/x-sv4cpio",           "sv4cpio"},
	{"application/x-tar",               "tar"},
	{"application/vnd.ms-powerpoint",   "ppt"},
	{"audio/basic",                     "au"},
	{"text/rtf",                        "rtf"},
	{"application/x-troff",             "t"},
	{"application/vnd.rn-realmedia",    "rm"},
	{"video/x-msvideo",                 "avi"},
	{"application/x-shockwave-flash",   "swf"},
	{"text/tab-separated-values",       "tsv"},
	{"application/CDFV2-unknown",       "unknown"},
	{"application/msword",              "doc"},
	{"application/vnd.ms-office",       "doc"},
	{"application/vnd.oasis.opendocument.text",                                 "odt"},
	{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "docx"},
	{"application/vnd.openxmlformats-officedocument.wordprocessingml",          "docx"},
	{"application/vnd.openxmlformats-officedocument.presentationml",            "pptx"},
	{"application/vnd.openxmlformats-officedocument.spreadsheetml",             "xlsx"},
	{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",       "xlsx"},
	{"application/vnd.adobe.apollo-application-installer-package+zip",          "air"}
};

std::string getDefaultExtension(const std::string& fileName) {
	return fileName.substr(fileName.find_last_of(".") + 1);
}

#if defined _WIN32 || defined _WIN64
	#include <windows.h>
	#include "magic.h"

	/*// Declare functions
	typedef void* (__stdcall *f_magic_open)(int);
	typedef int (__stdcall *f_magic_load)(void*, char*);
	typedef char* (__stdcall *f_magic_file)(void*, char*);
	typedef void (__stdcall *f_magic_close)(void*);*/

	int flags = 0x000010;  // Return mime string
	// flags |= 0x000400;  // Return the MIME encoding

	// Load DLL
	std::string dllDir = "files/libs";
	/*bool setDir = SetDllDirectoryA(dllDir.c_str());
	#if defined _M_IX86
		HINSTANCE hGetProcIDDLL = LoadLibraryA((dllDir + "/magic1.dll").c_str());
	#else
		HINSTANCE hGetProcIDDLL = LoadLibraryA((dllDir + "/magic1_64.dll").c_str());
	#endif
	bool dllError = !hGetProcIDDLL;

	// Resolve function address
	f_magic_open magicOpen   = (f_magic_open)GetProcAddress(hGetProcIDDLL, "magic_open");
	f_magic_load magicLoad   = (f_magic_load)GetProcAddress(hGetProcIDDLL, "magic_load");
	f_magic_file magicFile   = (f_magic_file)GetProcAddress(hGetProcIDDLL, "magic_file");
	f_magic_close magicClose = (f_magic_close)GetProcAddress(hGetProcIDDLL, "magic_close");

	// Init magic
	auto cookie = magicOpen(flags);*/

	bool setDir = SetDllDirectoryA(dllDir.c_str());
	auto cookie = magic_open(flags);

	std::string getFileExtension(const std::string& fileName) {
//auto cookie = magic_open(flags);
		magic_load(cookie, (char *)(dllDir + "/magic.mgc").c_str());
		const char* extension = magic_file(cookie, (char *)fileName.c_str());
		magic_close(cookie);

		/*if (dllError)
			return getDefaultExtension(fileName);
		// Load file
		auto status = magicLoad(cookie, (char *)"files/libs/libmagic");
		std::string extension = magicFile(cookie, (char *)fileName.c_str());
		magicClose(cookie);*/

		if (GetLastError() || extension == NULL ||
			MIME_EXTENSION_LIST.find(extension) == MIME_EXTENSION_LIST.end())
			return getDefaultExtension(fileName);
		else
			return MIME_EXTENSION_LIST.at(extension);
	}

#else
	#include <unistd.h>

	std::string getFileExtension(const std::string& fileName) {
		char buffer[128];
		std::string result;
		std::string cmd = "file --mime-type \""+ tools::PROGRAM_PATH +"/"+ fileName +"\"";

		// Get exclusive access to console
		{
			tools::LOCK lock(tools::MUTEX);

			std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
			if (!pipe)
				return getDefaultExtension(fileName);

			// Get command result
			while (!feof(pipe.get())) {
				if (fgets(buffer, 128, pipe.get()) != NULL)
					result += buffer;
			}
		}

		int pos = fileName.size() + 2;
		result  = result.substr(pos, result.size() - pos - 1);
		if (MIME_EXTENSION_LIST.find(result) != MIME_EXTENSION_LIST.end())
			return MIME_EXTENSION_LIST.at(result);
		else
			return getDefaultExtension(fileName);
	}

#endif

}  // End namespace
