/**
 * @brief   Interface for file extensions
 * @package fileext
 * @file    fileext.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    12.07.2016 -- 10.02.2018
 */
#include <fstream>

#include "../tools.hpp"

#include "fileext.hpp"


namespace fileext {

/** Config script file path */
const std::string LIB_PATH = tools::PROGRAM_PATH + "/files/libs";
const std::string SCRIPT_FILE = LIB_PATH + "/xpathconfig.min.js";

// public:
FileExtension::FileExtension(const std::string& fileName)
	: m_fileName(fileName) {}

void FileExtension::saveHtml(std::string dir, const std::string& fileName) const {
	auto node = m_htmlTree.child("html").child("head");
	// Add `head` tag
	if (!node)
		node = m_htmlTree.child("html").prepend_child("head");

	// Add encoding definition
	auto nd = node.append_child("meta");
	nd.append_attribute("http-equiv") = "content-type";
	nd.append_attribute("content")    = "text/html;charset=utf8";
	// Add basic element styles
	nd = node.append_child("style");
	nd.append_child(pugi::node_pcdata).set_value(
		"p{margin:0;} td{border:1px solid #efefff;} " \
		"table{border-collapse: collapse;} " \
		"body *:not(tr, td){display:block !important;}"
	);

	// Create dir if not exists
	dir += "/" + fileName;
	tools::createDir(dir);

	// Save images
	if (m_extractImages) {
		int i = 0;
		for (const auto& image : m_imageList) {
			// Save to file
			std::string imagePath = std::to_string(i + 1) + "." + image.second;
			std::ofstream imageFile(dir + "/" + imagePath, std::ios_base::binary);
			imageFile << image.first;
			imageFile.close();

			// Update `img` tags
			std::string xpath = "//img[@data-tag="+ std::to_string(i) +"]";
			auto imageNode = m_htmlTree.select_node(xpath.c_str());
			auto node = imageNode.node();
			node.append_attribute("src") = (imagePath).c_str();
			node.remove_attribute("data-tag");
			++i;
		}
	}

	std::ofstream outputFile(dir + "/"+ fileName, std::ios_base::binary);
	m_htmlTree.save(outputFile, "\t", pugi::format_no_empty_element_tags, pugi::encoding_auto);
	//m_htmlTree.save(outputFile, "\t", pugi::format_no_empty_element_tags, pugi::encoding_utf16_be);
}

void FileExtension::loadStyle(pugi::xml_node& node, const std::string& style) const {
	node.append_child("style").append_child(pugi::node_pcdata).set_value(style.c_str());
}

}  // End namespace
