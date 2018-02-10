/**
 * @brief   JSON files into HTML сonverter
 * @package json
 * @file    json.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    04.08.2017 -- 10.02.2018
 */
#include <fstream>

#include "../../tools.hpp"

#include "json.hpp"


namespace json {

/** Inline style */
const std::string STYLE = ".key,.value{display:inline-block}div{font-family:monospace;"
						  "font-size:13px}.content{margin-left:25px}.value{font-size:0}"
						  ".value span{font-size:13px}.key-data{color:#994500}"
						  ".value-data{color:#1a1aa6}";

// public:
Json::Json(const std::string& fileName)
	: FileExtension(fileName) {}

void Json::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	auto htmlTag = m_htmlTree.append_child("html");
	auto headTag = htmlTag.append_child("head");
	auto bodyTag = htmlTag.append_child("body");
	FileExtension::loadStyle(headTag, STYLE);

	nlohmann::json document;
	std::ifstream documentFile(m_fileName);
	documentFile >> document;
	documentFile.close();

	addBrackets("{", "}", document, bodyTag);
}


// private:
void Json::objectWalker(const nlohmann::json& object, pugi::xml_node& htmlNode) const {
	auto contentDiv = htmlNode.append_child("div");
	contentDiv.append_attribute("class") = "content";

	const auto& objectEnd = object.end();
	for (auto it = object.begin(); it != objectEnd; ++it) {
		const auto& value = it.value();
		const bool isLastElement = (++it == objectEnd);
		--it;  // Because nlohmann::json::iterator has no pointer arithmetic

		auto pairDiv = contentDiv.append_child("div");
		pairDiv.append_attribute("class") = "pair";

		// Add key (if exists)
		try {
			std::string key = it.key();
			auto keyDiv = pairDiv.append_child("div");
			keyDiv.append_attribute("class") = "key";

			auto keySpan = keyDiv.append_child("span");
			keySpan.append_attribute("class") = "key-data";
			keySpan.append_child(pugi::node_pcdata).set_value(("\""+ key +"\"").c_str());

			auto spacerSpan = keyDiv.append_child("span");
			spacerSpan.append_attribute("class") = "key-spacer";
			spacerSpan.append_child(pugi::node_pcdata).set_value(":");
		}
		catch (...) {}

		// Add value
		if (value.is_array()) {
			addBrackets("[", "]", value, pairDiv, !isLastElement);
		}
		else if (value.is_object()) {
			addBrackets("{", "}", value, pairDiv, !isLastElement);
		}
		else {
			auto valueDiv = pairDiv.append_child("div");
			valueDiv.append_attribute("class") = "value";

			auto valueSpan = valueDiv.append_child("span");
			valueSpan.append_attribute("class") = "value-data";

			std::string data;
			if (value.is_string())
				data = "\""+ value.get<std::string>() +"\"";
			else if (value.is_number())
				data = std::to_string(value.get<int>());
			else if (value.is_boolean())
				data = value.get<bool>() ? "true" : "false";
			else
				data = "null";
			valueSpan.append_child(pugi::node_pcdata).set_value(data.c_str());

			if (!isLastElement)
				addComma(valueDiv);
		}
	}
}

void Json::addBrackets(const std::string& bracketOpen, const std::string& bracketClose,
					   const nlohmann::json& object, pugi::xml_node& htmlNode,
					   const bool shouldAddComma) const
{
	auto bracketOpenDiv = htmlNode.append_child("div");
	bracketOpenDiv.append_attribute("class") = "bracket";
	bracketOpenDiv.append_child(pugi::node_pcdata).set_value(bracketOpen.c_str());

	objectWalker(object, htmlNode);

	auto bracketCloseDiv = htmlNode.append_child("div");
	bracketCloseDiv.append_attribute("class") = "bracket";
	bracketCloseDiv.append_child(pugi::node_pcdata).set_value(bracketClose.c_str());

	if (shouldAddComma)
		addComma(bracketCloseDiv);
}

void Json::addComma(pugi::xml_node& htmlNode) const {
	auto commaSpan = htmlNode.append_child("span");
	commaSpan.append_attribute("class") = "value-comma";
	commaSpan.append_child(pugi::node_pcdata).set_value(",");
}

}  // End namespace
