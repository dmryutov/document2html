/**
 * @brief     XML files into HTML сonverter
 * @package   xml
 * @file      xml.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @version   1.0
 * @date      04.08.2016 -- 18.10.2017
 */
#include <fstream>

#include "../../tools.hpp"

#include "xml.hpp"


namespace xml {

/** Specific style file path */
//const std::string STYLE_FILE = tools::PROGRAM_PATH + "/files/style/xmlStyle.min.css";
const std::string STYLE_FILE = "style.css";

// public:
Xml::Xml(const std::string& fileName)
	: FileExtension(fileName) {}

void Xml::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	auto htmlTag = m_htmlTree.append_child("html");
	auto headTag = htmlTag.append_child("head");
	auto bodyTag = htmlTag.append_child("body");
	FileExtension::loadStyle(headTag, STYLE_FILE);

	pugi::xml_document tree;
	tree.load_file(m_fileName.c_str());

	auto treeRoot = tree.document_element().parent();
	treeWalker(treeRoot, bodyTag);
}


// private:
void Xml::treeWalker(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	auto blockDiv = htmlNode.append_child("div");
	blockDiv.append_attribute("class") = "block";
	for (const auto& child : xmlNode) {
		if (child.type() == pugi::node_pcdata) {
			htmlNode.append_child(pugi::node_pcdata).set_value(child.value());
			continue;
		}

		auto lineDiv = blockDiv.append_child("div");
		lineDiv.append_attribute("class") = "line";

		auto tagSpan = lineDiv.append_child("span");
		tagSpan.append_attribute("class") = "tag";
		tagSpan.append_child(pugi::node_pcdata).set_value(("<"+ std::string(child.name())).c_str());

		for (const auto& attr : child.attributes()) {
			tagSpan.append_child(pugi::node_pcdata).set_value(" ");

			auto attrName = tagSpan.append_child("span");
			attrName.append_attribute("class") = "attribute-name";
			attrName.append_child(pugi::node_pcdata).set_value(attr.name());

			tagSpan.append_child(pugi::node_pcdata).set_value("=\"");

			auto attrValue = tagSpan.append_child("span");
			attrValue.append_attribute("class") = "attribute-value";
			attrValue.append_child(pugi::node_pcdata).set_value(attr.value());

			tagSpan.append_child(pugi::node_pcdata).set_value("\"");
		}

		tagSpan.append_child(pugi::node_pcdata).set_value(">");

		auto contentDiv = blockDiv.append_child("div");
		contentDiv.append_attribute("class") = "content";
		treeWalker(child, contentDiv);

		lineDiv = blockDiv.append_child("div");
		lineDiv.append_attribute("class") = "line";

		tagSpan = lineDiv.append_child("span");
		tagSpan.append_attribute("class") = "tag";
		tagSpan.append_child(pugi::node_pcdata)
			   .set_value(("</"+ std::string(child.name()) +">").c_str());
	}
	if (!blockDiv.first_child())
		blockDiv.parent().remove_child(blockDiv);
}

}  // End namespace