/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      excel.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#include <fstream>

#include "../../pugixml/pugixml.hpp"
#include "../../tools.hpp"

#include "book.hpp"
#include "xlsx.hpp"

#include "excel.hpp"


namespace excel {

/** Specific style file path */
//const std::string STYLE_FILE = tools::PROGRAM_PATH + "/files/style/excelStyle.min.css";
const std::string STYLE_FILE = "style.css";

// public:
Excel::Excel(const std::string& fileName, const std::string& extension)
	: FileExtension(fileName), m_extension(extension) {}

void Excel::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	auto htmlTag  = m_htmlTree.append_child("html");
	auto headTag  = htmlTag.append_child("head");
	auto bodyTag  = htmlTag.append_child("body");
	auto mainNode = bodyTag.append_child("div");
	mainNode.append_attribute("class") = "tabContent";
	FileExtension::loadStyle(headTag, STYLE_FILE);

	// Convert file
	Book* book = new Book(m_fileName, mainNode, m_addStyle, m_extractImages, m_mergingMode, m_imageList);
	if (m_extension == "xlsx") {
		Xlsx xlsx(book);
		xlsx.openWorkbookXlsx();
	}
	else {
		book->openWorkbookXls();
	}

	// Add tabs
	int sheetCount = book->m_sheetList.size();
	for (int i = 1; i <= sheetCount; ++i) {
		auto nd = bodyTag.insert_child_before("input", mainNode);
		nd.append_attribute("id") = ("tab" + std::to_string(i)).c_str();
		nd.append_attribute("type") = "radio";
		nd.append_attribute("name") = "tab";
		if (i == 1)
			nd.append_attribute("checked") = "checked";

		nd = bodyTag.insert_child_before("label", mainNode);
		nd.append_attribute("for") = ("tab" + std::to_string(i)).c_str();
		nd.append_attribute("id") = ("tabL" + std::to_string(i)).c_str();
		nd.append_child(pugi::node_pcdata).set_value(book->m_sheetNames[i-1].c_str());
	}
	auto nd = bodyTag.insert_child_before("div", mainNode);
	nd.append_attribute("style") = "clear:both";

	delete book;
}

}