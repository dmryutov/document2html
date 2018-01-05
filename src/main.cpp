/**
 * @brief   Document to HTML converter
 * @package document2html
 * @file    main.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0
 * @date    04.01.2018 -- 05.01.2018
 */
#include <iostream>
#include <string>

#include "libs/getoptpp/getoptpp.hpp"

#include "libs/fileext/archive/archive.hpp"
#include "libs/fileext/csv/csv.hpp"
#include "libs/fileext/doc/doc.hpp"
#include "libs/fileext/docx/docx.hpp"
#include "libs/fileext/epub/epub.hpp"
#include "libs/fileext/excel/excel.hpp"
#include "libs/fileext/html/html.hpp"
#include "libs/fileext/json/json.hpp"
#include "libs/fileext/odt/odt.hpp"
#include "libs/fileext/pdf/pdf.hpp"
#include "libs/fileext/ppt/ppt.hpp"
#include "libs/fileext/rtf/rtf.hpp"
#include "libs/fileext/txt/txt.hpp"
#include "libs/fileext/xml/xml.hpp"
#include "libs/pymagic/pymagic.hpp"
#include "libs/tools.hpp"

#if defined(_WIN32) || defined(_WIN64)
	#include "libs/dirent.h"
#else
	#include <dirent.h>
#endif


const std::string APP = "document2html";
const std::string VERSION = "1.0";


/**
 * @brief
 *     Convert single file
 * @param[in] input
 *     Name of input file
 * @param[in] output
 *     Name of output directory
 * @param[in] style
 *     True if should extract styles
 * @param[in] image
 *     True if should extract images
 * @since 1.0
 */
void convertFile(std::string input, std::string output, bool style, bool image);

/**
 * @brief
 *     Search files and convert them
 * @param[in] input
 *     Name of input directory
 * @param[in] output
 *     Name of output directory
 * @param[in] style
 *     True if should extract styles
 * @param[in] image
 *     True if should extract images
 * @since 1.0
 */
void convertFolder(std::string input, std::string output, bool style, bool image);


void convertFile(std::string input, std::string output, bool style, bool image) {
	size_t last = input.find_last_of("/");
	std::string name = input.substr(last + 1);
	std::string dir = input.substr(0, last);
	std::string ext = pymagic::getFileExtension(input);

	std::unique_ptr<fileext::FileExtension> document;
	try {
		if (ext == "docx")
			document.reset(new docx::Docx(input));
		else if (ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xht")
			document.reset(new html::Html(input));
		else if (ext == "xml")
			document.reset(new xml::Xml(input));
		else if (ext == "txt" || ext == "md" || ext == "markdown")
			document.reset(new txt::Txt(input));
		else if (ext == "json")
			document.reset(new json::Json(input));
		else if (ext == "doc")
			document.reset(new doc::Doc(input));
		else if (ext == "rtf")
			document.reset(new rtf::Rtf(input));
		else if (ext == "odt")
			document.reset(new odt::Odt(input));
		else if (ext == "xls" || ext == "xlsx")
			document.reset(new excel::Excel(input, ext));
		else if (ext == "csv")
			document.reset(new csv::Csv(input));
		else if (ext == "ppt")
			document.reset(new ppt::Ppt(input));
		else if (ext == "epub")
			document.reset(new epub::Epub(input));
		else if (ext == "pdf")
			document.reset(new pdf::Pdf(input));
		else if (ext == "zip" || ext == "rar" || ext == "tar" || ext == "gz" ||
				 ext == "bz2" || (tools::IS_WINDOWS && ext == "7z"))
		{
			std::string archive = input + ".archive";
			archive::extractArchive(dir, name, ext, archive);
			std::cout << "Archive extracted: " << input << std::endl;
			convertFolder(archive, output, style, image);
			return;
		}
		else {
			std::cout << "Unsupported file extension: " << ext << std::endl;
			return;
		}

		document->convert(style, image, 0);
		document->saveHtml(output, name +".html");
		std::cout << "Conversion complete: " << input << std::endl;
	}
	catch (...) {
		std::cerr << "Error: " << input << std::endl;
	}
	document.reset();

}

void convertFolder(std::string input, std::string output, bool style, bool image) {
	DIR *dp = dp = opendir(input.c_str());
	struct dirent *dirp;
	if (dp) {
		while ((dirp = readdir(dp))) {
			if (dirp->d_name[0] != '.') {
				std::string path = input +"/"+ dirp->d_name;
				if (tools::isDirectory(path))
					convertFolder(path, output, style, image);
				else
					convertFile(path, output, style, image);
			}
		}
		closedir(dp);
	}
	else {
		std::cerr << "Couldn't open folder: " << input << std::endl;
	}
}

int main(int argc, char* argv[]) {
	bool isFile, style, image, help, version;
	std::string input, output;

	try {
		GetOpt::GetOpt_pp ops(argc, argv);
		ops >> GetOpt::Option('f', "file", input);
		isFile = !input.empty();

		ops	>> GetOpt::Option('d', "dir",  input)
			>> GetOpt::Option('o', "out",  output)
			>> GetOpt::OptionPresent('s', "style",   style)
			>> GetOpt::OptionPresent('i', "image",   image)
			>> GetOpt::OptionPresent('h', "help",    help)
			>> GetOpt::OptionPresent('v', "version", version);

		if (help) {
			std::cout << "Usage: " << std::endl
					  << "\t" << APP << " -f|-d <input file|dir> -o <output dir> [-si]" << std::endl
					  << "\t" << APP << " -h|--help" << std::endl
					  << "\t" << APP << " -v|--version" << std::endl
					  << "Options:" << std::endl
					  << "\t" << "-f|--file"    << "\t" << "input file" << std::endl
					  << "\t" << "-d|--dir"     << "\t" << "input directory" << std::endl
					  << "\t" << "-o|--out"     << "\t" << "output directory" << std::endl
					  << "\t" << "-s|--style"   << "\t" << "extract styles" << std::endl
					  << "\t" << "-i|--image"   << "\t" << "extract images" << std::endl
					  << "\t" << "-h|--help"    << "\t" << "display help message" << std::endl
					  << "\t" << "-v|--version" << "\t" << "display package version" << std::endl
					  << std::endl;
			return 0;
		}
		if (version) {
			std::cout << APP << " version " << VERSION << std::endl;
			return 0;
		}
		else if (input.empty()) {
			std::cerr << "Input file/directory (-f|d) is required argument!" << std::endl;
			return 1;
		}
		else if (output.empty()) {
			std::cerr << "Output directory (-o) is required argument!" << std::endl;
			return 1;
		}
		else if (ops.options_remain()) {
			std::cerr << "Too many options!" << std::endl;
			return 1;
		}
		else if (!tools::fileExists(input)) {
			std::cerr << "Input file/directory does not exists!" << std::endl;
			return 1;
		}
	}
	catch (const GetOpt::GetOptEx& ex) {
		std::cerr << "Error in arguments!" << std::endl;
		return 1;
	}

	// Start convertsion
	input = tools::absolutePath(input);
	tools::createDir(output);
	if (isFile)
		convertFile(input, output, style, image);
	else
		convertFolder(input, output, style, image);

	return 0;
}
