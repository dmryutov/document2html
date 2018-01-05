/**
 * @brief     ODT files into HTML сonverter
 * @package   odt
 * @file      odt.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @date      12.08.2017 -- 18.10.2017
 */
#include <algorithm>
#include <fstream>

#include "../../tools.hpp"

#include "odt.hpp"


namespace odt {

/** Header tags map */
const std::unordered_map<std::string, std::string> HEADER_LIST {
	{"Title",    "h2"},
	{"Subtitle", "h3"},
	{"Normal",   "p"}
};
/** List type map */
const std::unordered_map<std::string, std::string> LIST_TYPE {
	{"1", "decimal"},
	{"0", "decimal-leading-zero"},
	{"I", "upper-roman"},
	{"i", "lower-roman"},
	{"A", "upper-alpha"},
	{"a", "lower-alpha"}
};
/** Border position list */
const std::vector<std::string> BORDER_LIST {
	"border",
	"border-top",
	"border-left",
	"border-right",
	"border-bottom"
};

// public:
Odt::Odt(const std::string& fileName)
	: FileExtension(fileName) {}

void Odt::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;
	auto mainNode   = m_htmlTree.append_child("html").append_child("body");

	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "content.xml", tree);
	getStyleMap(tree);
	getListStyleMap();

	for (const auto& node : tree.child("office:document-content")
								.child("office:body").child("office:text"))
	{
		buildElement(node, mainNode, false);
	}
}


// private:
// Loading styles
void Odt::getStyleMap(const pugi::xml_document& tree) {
	for (const auto& node : tree.child("office:document-content").child("office:automatic-styles")) {
		std::string key = node.attribute("style:name").value();
		// Get parent tag attributes
		for (const auto& attr : node.attributes()) {
			std::string attrName = attr.name();
			if (attrName != "style:name") {
				attrName = attrName.substr(attrName.find_first_of(":") + 1);
				m_styleMap[key][attrName] = attr.value();
			}
		}

		// Get children tags attributes
		for (const auto& child : node.children()) {
			for (const auto& attr : child.attributes()) {
				std::string attrName = attr.name();
				attrName = attrName.substr(attrName.find_first_of(":") + 1);
				m_styleMap[key][attrName] = attr.value();
			}
		}
	}
}

void Odt::getListStyleMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "styles.xml", tree);

	for (const auto& node : tree.select_nodes("//text:list-style")) {
		auto styleNode = node.node();
		std::string key = styleNode.attribute("style:name").value();

		// Get children tags attributes
		for (const auto& levelStyleNode : styleNode.children()) {
			std::string levelStyleNodeName = levelStyleNode.name();
			int level = levelStyleNode.attribute("text:level").as_int() - 1;
			if (levelStyleNodeName == "text:list-level-style-bullet")
				m_listStyleMap[key][level]["type"] = "ul";
			else
				m_listStyleMap[key][level]["type"] = "ol";

			// Get parent tag attributes
			for (const auto& attr : levelStyleNode.attributes()) {
				std::string attrName = attr.name();
				attrName = attrName.substr(attrName.find_first_of(":") + 1);
				m_listStyleMap[key][level][attrName] = attr.value();
			}

			// Get children tags attributes
			for (const auto& child : levelStyleNode.children()) {
				std::string childName = child.name();
				for (const auto& attr : child.attributes()) {
					std::string attrName = attr.name();
					attrName = attrName.substr(attrName.find_first_of(":") + 1);
					m_listStyleMap[key][level][attrName] = attr.value();
				}
			}
		}
	}
}

// Building elements
void Odt::buildElement(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode, bool inElement) {
	std::string nodeName = xmlNode.name();
	if (nodeName == "text:h") {
		auto h1 = htmlNode.append_child("h1");
		buildParagraph(xmlNode, h1);
	}
	else if (nodeName == "text:list") {
		buildList(xmlNode, htmlNode);
	}
	else if (nodeName == "table:table") {
		auto table = htmlNode.append_child("table");
		buildTable(xmlNode, table);
	}
	// `text:p` or other tags
	else {
		auto p = htmlNode;
		std::string tagName = HEADER_LIST.at(xmlNode.attribute("parent-style-name").value());
		if (tagName.empty())
			tagName = "p";

		if (!inElement || (tagName == "h2" || tagName == "h3"))  // (inE && (h2 || h3)) || !inE
			p = p.append_child(tagName.c_str());
		else if (inElement && htmlNode.first_child())
			p.append_child("br");
		buildParagraph(xmlNode, p);
	}
}

void Odt::buildParagraph(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	for (const auto& paragraphNode : xmlNode) {
		std::string paragraphNodeName = paragraphNode.name();

		if (paragraphNode.type() == pugi::node_pcdata) {
			buildPlainText(paragraphNode, htmlNode);
			continue;
		}
		else if (!(paragraphNodeName == "text:span" || paragraphNodeName == "draw:frame")) {
			continue;
		}

		for (const auto& child : paragraphNode) {
			std::string childName = child.name();
			if (child.type() == pugi::node_pcdata)
				buildPlainText(child, htmlNode);
			else if (childName == "text:line-break")
				htmlNode.append_child("br");
			else if (childName == "text:s")
				htmlNode.append_child(pugi::node_pcdata).set_value(" ");
			else if (childName == "text:tab")
				htmlNode.append_child(pugi::node_pcdata).set_value("\t");
			else if (childName == "text:a")
				buildHyperlink(child, htmlNode);
			else if (childName == "draw:image")
				buildImage(child, htmlNode);
		}
	}
}

void Odt::buildPlainText(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::string text = xmlNode.value();
	if (text.empty())
		return;

	// Wrap text with any modifiers it might have (bold/italics/underlined)
	if (m_addStyle)
		addStyle(xmlNode, htmlNode);
	htmlNode.append_child(pugi::node_pcdata).set_value(text.c_str());
}

void Odt::buildHyperlink(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	auto link = htmlNode.append_child("a");
	link.append_attribute("href") = xmlNode.attribute("xlink:href").value();
	buildParagraph(xmlNode, link);
}

void Odt::buildImage(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	if (!m_extractImages)
		return;

	// Load image data
	std::string path = xmlNode.attribute("xlink:href").value();
	std::string ext  = path.substr(path.find_last_of('.') + 1);
	std::string imageData;
	Ooxml::extractFile(m_fileName, path, imageData);
	m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

	// Add `data-tag` attribute and some style
	auto imageNode = htmlNode.append_child("img");
	imageNode.append_attribute("data-tag") = m_imageList.size() - 1;
	if (m_addStyle)
		addImageStyle(xmlNode.parent(), imageNode);
}

void Odt::buildTable(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	if (m_addStyle)
		addTableStyle(xmlNode);

	for (const auto& child : xmlNode.children("table:table-row")) {
		auto tr = htmlNode.append_child("tr");
		if (m_addStyle)
			addRowStyle(child, tr);
		buildTr(child, tr);
	}
	deleteMerging(htmlNode);
}

void Odt::buildTr(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	for (const auto& child : xmlNode.children("table:table-cell")) {
		auto td = htmlNode.append_child("td");
		if (m_addStyle)
			addCellStyle(child, td);

		for (const auto& tdContent : child)
			buildElement(tdContent, td, true);

		// if there is colspan then set it here
		int colspan = child.attribute("table:number-columns-spanned").as_int();
		int rowspan = child.attribute("table:number-rows-spanned").as_int();
		if (colspan > 1)
			td.append_attribute("colspan") = colspan;
		if (rowspan > 1)
			td.append_attribute("rowspan") = rowspan;
	}
}

void Odt::deleteMerging(pugi::xml_node& table) const {
	if (m_mergingMode == 0)
		return;

	for (auto tr = table.first_child(); tr; tr = tr.next_sibling()) {
		int count = 0;
		for (auto td = tr.first_child(); td; td = td.next_sibling(), ++count) {
			int rowspan = td.attribute("rowspan").as_int();
			int colspan = td.attribute("colspan").as_int();
			// Delete rowspan
			if (rowspan > 1) {
				auto next_row = tr.next_sibling();
				td.remove_attribute("rowspan");

				// Take into account next row colspans
				int j = 0;
				for (
					auto next_td = next_row.first_child();
					next_td && j < count;
					next_td = next_td.next_sibling(), ++j
				) {
					int cspan = next_td.attribute("colspan").as_int();
					if (cspan > 1)
						count -= cspan - 1;
				}

				auto nd = (count > 0)
						  ? next_row.insert_copy_after(td, *std::next(next_row.begin(), count-1))
						  : next_row.insert_copy_before(td, next_row.first_child());
				nd.append_attribute("rowspan") = rowspan-1;
				if (m_mergingMode == 2)
					tools::xmlDeleteAllChildren(nd);
			}
			// Delete colspan
			if (colspan > 1) {
				td.remove_attribute("colspan");
				for (int i = 1; i < colspan; ++i) {
					auto nd = tr.insert_copy_after(td, td);
					if (m_mergingMode == 2)
						tools::xmlDeleteAllChildren(nd);
				}
			}
		}
	}
}

void Odt::buildList(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::string key;
	int level = getIndentationLevel(xmlNode, key);

	// Get list type
	std::string tagName = m_listStyleMap[key][level]["type"];
	auto ul = htmlNode.append_child(tagName.c_str());
	if (tagName == "ol") {
		std::string s = LIST_TYPE.at(m_listStyleMap[key][level]["num-format"]);
		ul.append_attribute("style") = ("list-style-type: "+ s +";").c_str();
	}

	for (const auto& listNode : xmlNode) {
		auto li = ul.append_child("li");
		for (const auto& child : listNode)
			buildElement(child, li, true);
	}
}

int Odt::getIndentationLevel(pugi::xml_node xmlNode, std::string& key) const {
	key = xmlNode.attribute("text:style-name").value();
	int level = 0;
	while (key.empty()) {
		xmlNode = xmlNode.parent().parent();
		key = xmlNode.attribute("text:style-name").value();
		level++;
	}
	return level;
}

// Style
void Odt::addTextStyle(pugi::xml_node& node, const std::string& style) const {
	std::string lastChildName = node.last_child().name();
	if (lastChildName == style)
		node = node.last_child();
	else
		node = node.append_child(style.c_str());
}

void Odt::addStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::unordered_map<std::string, std::string> parentStyleMap;
	std::unordered_map<std::string, std::string> elementStyleMap;
	std::string parentKey  = xmlNode.parent().parent().attribute("text:style-name").value();
	std::string elementKey = xmlNode.parent().attribute("text:style-name").value();
	auto& currentElementStyle = m_styleMap.at(elementKey);
	auto& currentParentStyle  = m_styleMap.at(parentKey);
	std::string value;
	auto node = htmlNode;

	// Add text style tags such as b/i/u/...
	if (currentElementStyle["font-weight"] == "bold")
		addTextStyle(htmlNode, "b");
	if (currentElementStyle["font-style"] == "italic")
		addTextStyle(htmlNode, "i");
	if (!currentElementStyle["text-underline-style"].empty())
		addTextStyle(htmlNode, "u");
	if (!currentElementStyle["text-line-through-style"].empty())
		addTextStyle(htmlNode, "s");

	value = currentElementStyle["text-position"].substr(0, 3);
	if (value == "sub")
		addTextStyle(htmlNode, "sub");
	else if (value == "sup")
		addTextStyle(htmlNode, "sup");

	// Get style for "style" attribute
	value = currentParentStyle["text-align"];
	if (!value.empty()) parentStyleMap["text-align"] = value;

	value = currentParentStyle["padding"];
	if (!value.empty()) parentStyleMap["padding"] = value;
	value = currentParentStyle["padding-left"];
	if (!value.empty()) parentStyleMap["padding-left"] = value;
	value = currentParentStyle["padding-right"];
	if (!value.empty()) parentStyleMap["padding-right"] = value;
	value = currentParentStyle["padding-top"];
	if (!value.empty()) parentStyleMap["padding-top"] = value;
	value = currentParentStyle["padding-bottom"];
	if (!value.empty()) parentStyleMap["padding-bottom"] = value;

	/*value = currentParentStyle["margin"];
	if (!value.empty()) parentStyleMap["margin"] = value;
	value = currentParentStyle["margin-left"];
	if (!value.empty()) parentStyleMap["margin-left"] = value;
	value = currentParentStyle["margin-right"];
	if (!value.empty()) parentStyleMap["margin-right"] = value;
	value = currentParentStyle["margin-top"];
	if (!value.empty()) parentStyleMap["margin-top"] = value;
	value = currentParentStyle["margin-bottom"];
	if (!value.empty()) parentStyleMap["margin-bottom"] = value;*/

	addGeneralStyle(parentKey, parentStyleMap);
	addGeneralStyle(elementKey, elementStyleMap);

	for (const auto& border : BORDER_LIST) {
		value = currentParentStyle[border];
		if (!value.empty())
			parentStyleMap[border] = value;
	}

	// Add parent style
	std::string style;
	for (const auto& sm : parentStyleMap)
		style += sm.first + ":" + sm.second + "; ";
	std::string styleAttr = htmlNode.attribute("style").value();
	if (!style.empty() &&
		styleAttr.substr(std::max(0, (int)(styleAttr.size()-style.size()))) != style
	) {
		if (styleAttr.empty())
			node.append_attribute("style") = style.c_str();
		else
			node.attribute("style").set_value((styleAttr + style).c_str());
	}

	// Add element style
	style.clear();
	for (const auto& sm : elementStyleMap) {
		if (sm.second != parentStyleMap[sm.first])
			style += sm.first + ":" + sm.second + "; ";
	}
	if (!style.empty()) {
		htmlNode = htmlNode.append_child("span");
		htmlNode.append_attribute("style") = style.c_str();
	}
}

void Odt::addGeneralStyle(const std::string& key,
						  std::unordered_map<std::string, std::string>& styleMap)
{
	auto& currentNodeStyle = m_styleMap.at(key);
	std::string value;
	value = currentNodeStyle["font-name"];
	if (!value.empty()) styleMap["font-family"] = "'"+ value +"'";

	value = currentNodeStyle["font-size"];
	if (!value.empty()) styleMap["font-size"] = value;

	value = currentNodeStyle["color"];
	if (!value.empty()) styleMap["color"] = value;

	value = currentNodeStyle["background-color"];
	if (!value.empty()) styleMap["background-color"] = value;

	if (currentNodeStyle["display"] == "none")
		styleMap["display"] = "none";
}

void Odt::addImageStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	std::string width  = xmlNode.attribute("svg:width").value();
	std::string height = xmlNode.attribute("svg:height").value();
	std::string style = "width:" + width + "; height:" + height + ";";
	htmlNode.append_attribute("style") = style.c_str();
}

void Odt::addTableStyle(const pugi::xml_node& xmlNode) {
	std::string key = xmlNode.attribute("table:style-name").value();
	auto& currentNodeStyle = m_styleMap[key];
	m_borderMap.clear();

	for (const auto& border : BORDER_LIST) {
		std::string value = currentNodeStyle[border];
		if (!value.empty())
			m_borderMap[border] = value;
	}
}

void Odt::addRowStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::string key = xmlNode.attribute("table:style-name").value();

	std::string value = m_styleMap[key]["min-row-height"];
	if (!value.empty()) {
		value = "height:" + value;
		htmlNode.append_attribute("style") = value.c_str();
	}
}

void Odt::addCellStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::unordered_map<std::string, std::string> styleMap;
	std::string key = xmlNode.attribute("table:style-name").value();
	std::string value;
	auto& currentNodeStyle = m_styleMap.at(key);

	value = currentNodeStyle["vertical-align"];
	if (!value.empty()) styleMap["vertical-align"] = value;

	value = currentNodeStyle["background-color"];
	if (!value.empty()) styleMap["background"] = value;

	for (const auto& border : BORDER_LIST) {
		value = currentNodeStyle[border];
		if (!value.empty())
			styleMap[border] = value;
		else if (!m_borderMap.at(border).empty())
			styleMap[border] = m_borderMap.at(border);
		else if (border != "border")
			styleMap[border] = "1px none #000";
	}

	std::string style;
	for (const auto& sm : styleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (!style.empty())
		htmlNode.append_attribute("style") = style.c_str();
}

}  // End namespace