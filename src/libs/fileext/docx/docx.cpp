/**
 * @brief     DOCX files into HTML сonverter
 * @package   docx
 * @file      docx.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright PolicyStat (https://github.com/PolicyStat/docx2html)
 * @date      12.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <fstream>

#include "docx.hpp"


namespace docx {

/** Header tags map */
const std::unordered_map<std::string, std::string> HEADER_LIST {
	{"heading 1",  "h1"},
	{"heading 2",  "h2"},
	{"heading 3",  "h3"},
	{"heading 4",  "h4"},
	{"heading 5",  "h5"},
	{"heading 6",  "h6"},
	{"heading 7",  "h6"},
	{"heading 8",  "h6"},
	{"heading 9",  "h6"},
	{"heading 10", "h6"}
};
/** List type map */
const std::unordered_map<std::string, std::string> LIST_TYPE {
	{"decimal",      "decimal"},
	{"decimalZero",  "decimal-leading-zero"},
	{"upperRoman",   "upper-roman"},
	{"lowerRoman",   "lower-roman"},
	{"upperLetter",  "upper-alpha"},
	{"lowerLetter",  "lower-alpha"},
	{"ordinal",      "decimal"},
	{"cardinalText", "decimal"},
	{"ordinalText",  "decimal"}
};
/** Only these tags contain text that we care about (e.g. don't care about delete tags) */
const std::vector<std::string> CONTENT_TAGS {
	"w:r",
	"w:hyperlink",
	"w:ins",
	"w:smartTag"
};
/** Horizontal align map */
const std::unordered_map<std::string, std::string> HORZ_ALIGN {
	{"left",   "left"},
	{"center", "center"},
	{"right",  "right"},
	{"both",   "justify"},
};
/** Vertical align map */
const std::unordered_map<std::string, std::string> VERT_ALIGN {
	{"top",         "top"},
	{"center",      "middle"},
	{"bottom",      "bottom"},
	{"justify",     "middle"},
	{"distributed", "middle"}
};
/** Border type map */
const std::unordered_map<std::string, std::string> BORDER_TYPE {
	{"",                 "none"},
	{"single",           "solid"},
	{"thin",             "solid"},
	{"medium",           "solid"},
	{"dashed",           "dashed"},
	{"dotted",           "dotted"},
	{"thick",            "solid"},
	{"double",           "double"},
	{"hair",             "dotted"},
	{"mediumDashed",     "dashed"},
	{"dashDot",          "dashed"},
	{"mediumDashDot",    "dashed"},
	{"dashDotDot",       "dotted"},
	{"mediumDashDotDot", "dotted"},
	{"slantDashDot",     "dashed"}
};
/** Border size map */
const std::unordered_map<std::string, int> BORDER_SIZE {
	{"",                 1},
	{"single",           1},
	{"thin",             1},
	{"medium",           2},
	{"dashed",           1},
	{"dotted",           1},
	{"thick",            3},
	{"double",           1},
	{"hair",             1},
	{"mediumDashed",     2},
	{"dashDot",          1},
	{"mediumDashDot",    2},
	{"dashDotDot",       1},
	{"mediumDashDotDot", 2},
	{"slantDashDot",     3}
};
/** Border position list */
const std::vector<std::string> BORDER_LIST {
	"top",
	"left",
	"right",
	"bottom"
};

// public:
Docx::Docx(const std::string& fileName)
	: FileExtension(fileName) {}

void Docx::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;
	auto mainNode   = m_htmlTree.append_child("html").append_child("body");

	getNumberingMap();
	getStyleMap();
	getRelationshipMap();

	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/document.xml", tree);

	for (const auto& node : tree.child("w:document").child("w:body")) {
		// Lists are handled specific => could double visit certain elements. Keep track
		// of visited elements and skip any that have been visited already
		std::string nodeName = node.name();
		if (nodeName == "w:sectPr" ||
			find(m_visitedNodeList.begin(), m_visitedNodeList.end(), node) !=
				m_visitedNodeList.end()
		)
			continue;
		std::string headerValue = isHeader(node);
		if (!headerValue.empty()) {
			auto header = mainNode.append_child(headerValue.c_str());
			getParagraphText(node, header);
			if (!header.first_child())
				header.parent().remove_child(header);
		}
		else if (nodeName == "w:p") {
			// Certain `p` tags denoted as `Title` tags. Strip out them
			auto pStyle = node.select_node(".//w:pStyle");
			std::string isTitle = pStyle.node().attribute("w:val").value();
			if (isTitle == "Title")
				continue;
			// Parse out the needed info from node
			if (isLi(node)) {
				buildList(node, mainNode);
			}
			// Handle generic `p` tag
			else {
				auto p = mainNode.append_child("p");
				getParagraphText(node, p);
				if (!p.first_child())
					p.parent().remove_child(p);
			}
		}
		else if (nodeName == "w:tbl") {
			auto table = mainNode.append_child("table");
			buildTable(node, table);
			continue;
		}
		m_visitedNodeList.emplace_back(node);
	}
}


// private:
void Docx::getNumberingMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/numbering.xml", tree);

	std::unordered_map<std::string, std::string> numIdList;
	// Each list type is assigned an abstractNumber that defines how lists should look
	for (const auto& node : tree.select_nodes("//w:num")) {
		auto nd = node.node();
		std::string abstractNumber = nd.child("w:abstractNumId").attribute("w:val").value();
		numIdList[abstractNumber] = nd.attribute("w:numId").value();
	}

	for (const auto& node : tree.select_nodes("//w:abstractNum")) {
		auto nd = node.node();
		std::string abstractNumId = nd.attribute("w:abstractNumId").value();
		// If we find abstractNumber that is not being used in document => ignore it
		if (numIdList.find(abstractNumId) == numIdList.end())
			continue;
		// Get level of abstract number
		for (const auto& child : nd.children("w:lvl")) {
			// Based on list type and ilvl (indentation level) store needed style
			m_numberingMap[numIdList[abstractNumId]].emplace_back(
				child.child("w:numFmt").attribute("w:val").value()
			);
		}
	}
}

void Docx::getStyleMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/styles.xml", tree);

	// This is a partial document and actual H1 is the document title, which
	// will be displayed elsewhere
	for (const auto& node : tree.select_nodes("//w:style")) {
		auto nd = node.node();
		std::unordered_map<std::string, std::string> style {
			{"header",    ""},
			{"font_size", ""},
			{"based_on",  ""}
		};
		// Get header info
		auto name = nd.child("w:name");
		if (!name)
			continue;
		std::string value = name.attribute("w:val").value();
		transform(value.begin(), value.end(), value.begin(), ::tolower);
		if (HEADER_LIST.find(value) != HEADER_LIST.end())
			style["header"] = HEADER_LIST.at(value);

		// Get size info
		auto rPr = nd.child("w:rPr");
		if (!rPr)
			continue;
		auto size = rPr.child("w:sz");
		if (size)
			style["font_size"] = size.attribute("w:val").value();

		// Get based on info
		auto basedOn = nd.child("w:basedOn");
		if (basedOn)
			style["based_on"] = basedOn.attribute("w:val").value();

		std::string styleId = nd.attribute("w:styleId").value();
		m_styleMap[styleId] = style;
	}
}

void Docx::getRelationshipMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/_rels/document.xml.rels", tree);

	for (const auto& node : tree.child("Relationships")) {
		auto id = node.attribute("Id").value();
		if (id)
			m_relationshipMap[id] = node.attribute("Target").value();
	}
}


std::string Docx::isHeader(const pugi::xml_node& node) const {
	if (isTopLevel(node))
		return "h2";
	std::string el_isNaturalHeader = isNaturalHeader(node);
	if (!el_isNaturalHeader.empty())
		return el_isNaturalHeader;
	if (hasIndentationLevel(node))
		return "";
	if (strcmp(node.name(), "w:tbl") == 0)
		return "";
	return "";
}

std::string Docx::isNaturalHeader(const pugi::xml_node& node) const {
	auto pPr = node.child("w:pPr");
	if (!pPr)
		return "";
	auto pStyle = pPr.child("w:pStyle");
	if (!pStyle)
		return "";
	std::string styleId = pStyle.attribute("w:val").value();
	if (m_styleMap.at(styleId).at("header") != "false")
		return m_styleMap.at(styleId).at("header");
	return "";
}

std::string Docx::getNumberingId(const pugi::xml_node& node) const {
	auto numId = node.select_node(".//w:numId");
	return numId.node().attribute("w:val").value();
}

bool Docx::hasIndentationLevel(const pugi::xml_node& node) const {
	auto ilvl = node.select_nodes(".//w:numPr/w:ilvl");
	return (ilvl.end() - ilvl.begin() != 0);
}

// Paragraph
void Docx::getParagraphText(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	for (const auto& child : xmlNode) {
		std::string childName = child.name();
		if (find(CONTENT_TAGS.begin(), CONTENT_TAGS.end(), childName) != CONTENT_TAGS.end()) {
			// Hyperlinks and insert tags need to be handled differently than r and smart tags
			if (childName == "w:r")
				getElementText(child, htmlNode);
			else if (childName == "w:hyperlink")
				buildHyperlink(child, htmlNode);
			else
				getParagraphText(child, htmlNode);
		}
	}
}

void Docx::getElementText(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	for (const auto& child : xmlNode) {
		std::string childName = child.name();
		if (childName == "w:t") {
			// Generate string data that for this particular `t` tag
			std::string text = child.child_value();
			if (text.empty())
				return;

			// Wrap text with any modifiers it might have (bold/italics/underlined)
			auto node = htmlNode;
			if (m_addStyle)
				addStyle(xmlNode, node);

			node.append_child(pugi::node_pcdata).set_value(text.c_str());
		}
		else if (childName == "w:br") {
			htmlNode.append_child("br");
		}
		else if (childName == "w:tab") {
			htmlNode.append_child(pugi::node_pcdata).set_value("\t");
		}
		else if (childName == "w:pict" || childName == "w:drawing") {
			buildImage(child, htmlNode);
		}
	}
}

// Hyperlink
void Docx::buildHyperlink(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	// If we have hyperlink we need to get relationship id
	auto hyperlinkId = xmlNode.attribute("r:id").value();

	// Once we have hyperlinkId then we need to replace hyperlink tag with its child run tags
	if (m_relationshipMap.find(hyperlinkId) != m_relationshipMap.end()) {
		auto link = htmlNode.append_child("a");
		link.append_attribute("href") = m_relationshipMap[hyperlinkId].c_str();
		getParagraphText(xmlNode, link);
	}
}

// Image
std::string Docx::getImageId(const pugi::xml_node& node) const {
	for (const auto& child : node.select_nodes(".//a:blip")) {
		auto id = child.node().attribute("r:embed").value();
		if (id)
			return id;
	}
	return "";
}

void Docx::getImageSize(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	auto child = xmlNode.select_node(".//a:xfrm").node().child("a:ext");
	if (!child)
		return;

	int width  = child.attribute("cx").as_int() / 9525;  // EMUS_PER_PIXEL
	int height = child.attribute("cy").as_int() / 9525;  // EMUS_PER_PIXEL

	std::string style = "width: " + std::to_string(width) + "px;";
	style += "height: " + std::to_string(height) + "px;";
	htmlNode.append_attribute("style") = style.c_str();
}

void Docx::buildImage(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	if (!m_extractImages)
		return;

	std::string imageId = getImageId(xmlNode);
	// This image does not have image id
	if (m_relationshipMap.find(imageId) == m_relationshipMap.end())
		return;
	std::string path = "word/" + m_relationshipMap[imageId];

	// Load image
	std::string ext = path.substr(path.find_last_of('.') + 1);
	std::string imageData;
	Ooxml::extractFile(m_fileName, path, imageData);
	m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

	// Add image node
	auto imageNode = htmlNode.append_child("img");
	imageNode.append_attribute("data-tag") = m_imageList.size() - 1;

	// Add style
	if (m_addStyle)
		getImageSize(xmlNode, imageNode);
}

// Table
void Docx::buildTable(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	if (m_addStyle)
		addTableStyle(xmlNode);

	for (const auto& child : xmlNode.children("w:tr")) {
		auto tr = htmlNode.append_child("tr");
		if (m_addStyle)
			addRowStyle(child, tr);

		buildTr(child, tr);
	}
}

void Docx::buildTr(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	int colIndex = 0;
	for (const auto& child : xmlNode.children("w:tc")) {
		if (find(m_visitedNodeList.begin(), m_visitedNodeList.end(), child) != m_visitedNodeList.end())
			continue;
		// Keep track of m_visitedNodeList
		m_visitedNodeList.emplace_back(xmlNode);

		// vMerge is what docx uses to denote that table cell is part of rowspan. First
		// cell has vMerge - start of rowspan, and vMerge will be denoted with `restart`.
		// If it is anything other than restart then it is continuation of another rowspan
		auto vMerge = child.child("w:tcPr").child("w:vMerge");
		std::string vMergeVal = vMerge.attribute("w:val").value();
		if (vMerge && vMergeVal != "restart" && m_mergingMode == 0)
			continue;

		auto td = htmlNode.append_child("td");
		if (m_addStyle)
			addCellStyle(child, td);

		// Loop through each and build list of all content
		bool needNewLine = false;
		for (const auto& tdContent : child) {
			// Since we are doing look-a-heads in this loop we need to check already visited nodes
			if (find(m_visitedNodeList.begin(), m_visitedNodeList.end(), tdContent) !=
				m_visitedNodeList.end()
			)
				continue;

			std::string tdContentName = tdContent.name();
			// Check to see if it is list or regular paragraph
			// If it is a list, create list and update m_visitedNodeList
			if (isLi(tdContent)) {
				buildList(tdContent, td);
			}
			else if (tdContentName == "w:tbl") {
				auto nestedTable = td.append_child("table");
				buildTable(tdContent, nestedTable);
			}
			// Do nothing
			else if (tdContentName == "w:tcPr") {
				m_visitedNodeList.push_back(tdContent);
				continue;
			}
			else {
				if (needNewLine)
					td.append_child("br");
				else
					needNewLine = true;
				getParagraphText(tdContent, td);
			}
		}

		// if there is colspan then set it here
		int colspan = getColspan(child);
		if (m_mergingMode == 0) {
			if (colspan > 1)
				td.append_attribute("colspan") = colspan;
			// If this td has a vMerge and it is restart then set rowspan here
			if (vMerge && vMergeVal == "restart")
				td.append_attribute("rowspan") = getRowspan(child);
		}
		else {
			// Rowspan
			if (m_mergingMode == 1 && vMerge && vMergeVal != "restart") {
				auto prevTd = std::next(htmlNode.previous_sibling().children("td").begin(), colIndex);
				for (const auto& nd : prevTd->children())
					td.append_copy(nd);
				// Add style
				if (m_addStyle) {
					td.remove_attribute("style");
					td.append_copy(prevTd->attribute("style"));
				}
			}
			// Colspan
			for (int i = 1; i < colspan; ++i) {
				auto newTd = htmlNode.append_child("td");
				if (m_mergingMode == 1) {
					for (const auto& nd : newTd.previous_sibling())
						newTd.append_copy(nd);
				}
				// Add style
				if (m_addStyle)
					newTd.append_copy(newTd.previous_sibling().attribute("style"));

				colIndex++;
			}
		}
		colIndex++;
	}
}

int Docx::getRowspan(const pugi::xml_node& node) const {
	int rowspan  = 1;
	int position = 1;
	// Find position of cell in row
	for (auto td = node.previous_sibling("w:tc"); td; td = td.previous_sibling("w:tc")) {
		int colspan = getColspan(td);
		position   += (colspan == 0) ? 1 : colspan;
	}

	for (auto tr = node.parent().next_sibling(); tr; tr = tr.next_sibling("w:tr")) {
		auto td = tr.child("w:tc");
		// Shift to required cell with number = `position`
		for (int i = 1; i < position; ) {
			int colspan = getColspan(td);
			i += (colspan == 0) ? 1 : colspan;

			td = td.next_sibling("w:tc");
		}

		if (!td.child("w:tcPr").child("w:vMerge"))
			break;
		++rowspan;
	}
	return rowspan;
}

int Docx::getColspan(const pugi::xml_node& node) const {
	auto gridSpan = node.child("w:tcPr").child("w:gridSpan");
	return gridSpan.attribute("w:val").as_int();
}

// List
void Docx::buildList(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	// Need to keep track of all incomplete nested lists
	std::vector<pugi::xml_node> nestedLists;
	// Need to keep track of current indentation level
	int currentIndentationLevel = -1;
	// Need to keep track of current list id
	std::string currentListId = "-1";
	// Need to keep track of list that new `li` tags should be added too
	auto currentList = htmlNode;
	pugi::xml_node node;

	// Get consecutive `li` tags that have content
	std::vector<pugi::xml_node> liNodes;
	getListNodes(xmlNode, liNodes);
	nestedLists.emplace_back(htmlNode);

	for (const auto& li : liNodes) {
		if (!isLi(li)) {
			// Get content and visited nodes
			buildNonListContent(li, htmlNode);
			m_visitedNodeList.emplace_back(li);
			continue;
		}

		int ilvl = getIndentationLevel(li);
		std::string numId = getNumberingId(li);
		std::string listType = m_numberingMap[numId][ilvl];
		if (listType.empty())
			listType = "decimal";

		// If ilvl is greater than current indentation level or list id is changing then we have
		// first li tag in nested list. Need to create new list object and update all variables
		if (ilvl > currentIndentationLevel || numId != currentListId) {
			if (listType == "bullet") {
				node = currentList.append_child("ul");
			}
			else {
				std::string style = "list-style-type:"+ LIST_TYPE.at(listType) +";";
				node = currentList.append_child("ol");
				node.append_attribute("style") = style.c_str();
				//node.append_attribute("data-list-type") = LIST_TYPE[listType].c_str();
			}
			currentIndentationLevel = ilvl;
			currentListId           = numId;
			nestedLists.emplace_back(node);
		}
		// Remove node from stack
		if (ilvl < currentIndentationLevel)
			nestedLists.pop_back();

		// Add li element to tree
		node = nestedLists.back().append_child("li");
		getParagraphText(li, node);
		currentList = node;

		m_visitedNodeList.push_back(li);
	}
}

void Docx::getListNodes(const pugi::xml_node& node, std::vector<pugi::xml_node>& liNodes) const {
	liNodes.emplace_back(node);
	std::string currentNumId  = getNumberingId(node);
	int startIndentationLevel = getIndentationLevel(node);
	for (auto li = node; li; li = li.next_sibling()) {
		if (!li.child_value())
			continue;

		// Stop lists if come across list item that should be heading
		if (isTopLevel(li))
			break;
		bool isListItem = isLi(li);
		if (isListItem && (startIndentationLevel > getIndentationLevel(li)))
			break;

		std::string numId = getNumberingId(li);
		// Not `p` tag or list item
		if (numId.empty() || numId == "-1") {
			liNodes.emplace_back(li);
			continue;
		}
		// If list id of next tag is different that previous that means new list (not nested)
		if (currentNumId != numId) {
			break;
		}
		if (isListItem && isLastLi(li, currentNumId)) {
			liNodes.emplace_back(li);
			break;
		}
		liNodes.emplace_back(li);
	}
}

int Docx::getIndentationLevel(const pugi::xml_node& node) const {
	auto ilvl = node.select_node(".//w:ilvl");
	if (!ilvl)
		return -1;
	return ilvl.node().attribute("w:val").as_int();
}

bool Docx::isLi(const pugi::xml_node& node) const {
	if (!isHeader(node).empty())
		return false;
	return hasIndentationLevel(node);
}

bool Docx::isTopLevel(const pugi::xml_node& node) const {
	int ilvl = getIndentationLevel(node);
	if (ilvl != 0)
		return false;
	return (m_numberingMap.at(getNumberingId(node))[ilvl] == "upperRoman");
}

bool Docx::isLastLi(const pugi::xml_node& node, const std::string& currentNumId) const {
	for (auto li = node; li; li = li.next_sibling()) {
		if (!isLi(li))
			continue;

		if (currentNumId != getNumberingId(li))
			return true;
		// If here, we have found another list item in current list, so `li` is not last
		return false;
	}
	// If we run out of element this must be last list item
	return true;
}

void Docx::buildNonListContent(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::string nodeName = xmlNode.name();
	if (nodeName == "w:tbl") {
		auto table = htmlNode.append_child("table");
		buildTable(xmlNode, table);
	}
	else if (nodeName == "w:p") {
		auto p = htmlNode.append_child("p");
		getParagraphText(xmlNode, p);
	}
}

// Style
bool Docx::hasStyle(const pugi::xml_node& node, const std::string& style) const {
	auto child = node.child(style.c_str());
	std::string childValue = child.attribute("w:val").value();
	return (child && childValue != "false");
}

std::string Docx::getStyleValue(const pugi::xml_node& parentNode, const std::string& nodeName,
								const std::string& styleName) const
{
	auto node = parentNode.child(nodeName.c_str());
	return node.attribute(styleName.c_str()).value();
}

void Docx::addTextStyle(pugi::xml_node& node, const std::string& style) const {
	std::string lastChildName = node.last_child().name();
	if (lastChildName == style)
		node = node.last_child();
	else
		node = node.append_child(style.c_str());
}

void Docx::addStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	std::unordered_map<std::string, std::string> parentStyleMap;
	std::unordered_map<std::string, std::string> elementStyleMap;
	std::string value;
	auto node = htmlNode;
	auto parentNode  = xmlNode.parent().child("w:pPr");
	auto elementNode = xmlNode.child("w:rPr");

	// Add text style tags such as b/i/u/...
	if (hasStyle(elementNode, "w:b"))
		addTextStyle(htmlNode, "b");
	if (hasStyle(elementNode, "w:i"))
		addTextStyle(htmlNode, "i");
	if (hasStyle(elementNode, "w:u"))
		addTextStyle(htmlNode, "u");
	if (hasStyle(elementNode, "w:strike") || hasStyle(elementNode, "w:dstrike"))
		addTextStyle(htmlNode, "s");

	value = getStyleValue(elementNode, "w:vertAlign", "w:val");
	if (value == "subscript")
		addTextStyle(htmlNode, "sub");
	else if (value == "superscript")
		addTextStyle(htmlNode, "sup");

	// Get style for "style" attribute
	value = getStyleValue(parentNode, "w:jc", "w:val");
	if (!value.empty())
		parentStyleMap["text-align"] = HORZ_ALIGN.at(value);

	value = getStyleValue(parentNode, "w:spacing", "w:before");
	if (!value.empty() && value != "auto")
		parentStyleMap["padding-left"] = std::to_string(stoi(value)/20) +"px";

	value = getStyleValue(parentNode, "w:spacing", "w:after");
	if (!value.empty() && value != "auto")
		parentStyleMap["padding-right"] = std::to_string(stoi(value)/20) +"px";

	addGeneralStyle(parentNode.child("w:rPr"), parentStyleMap);
	addGeneralStyle(elementNode, elementStyleMap);

	parentNode = parentNode.child("w:pBdr");
	for (const auto& border : BORDER_LIST) {
		std::string type  = getStyleValue(parentNode, "w:"+border, "w:val");
		std::string color = getStyleValue(parentNode, "w:"+border, "w:color");
		if (!type.empty()) {
			parentStyleMap["border-"+border] = std::to_string(BORDER_SIZE.at(type)) +"px "+
											   BORDER_TYPE.at(type) +" #"+
											   ((color.empty() || color == "auto") ? "000" : color);
		}
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

void Docx::addGeneralStyle(const pugi::xml_node& node,
						   std::unordered_map<std::string, std::string>& styleMap) const
{
	std::string value;
	value = getStyleValue(node, "w:rFonts", "w:ascii");
	if (!value.empty())
		styleMap["font-family"] = "'"+ value +"'";

	value = getStyleValue(node, "w:sz", "w:val");
	if (!value.empty())
		styleMap["font-size"] = std::to_string(stoi(value)/2) +"px";

	value = getStyleValue(node, "w:color", "w:val");
	if (!value.empty() && value != "auto")
		styleMap["color"] = "#"+ value;

	value = getStyleValue(node, "w:shd", "w:fill");
	if (!value.empty() && value != "auto")
		styleMap["background"] = "#"+ value;

	if (hasStyle(node, "vanish"))
		styleMap["display"] = "none";

	if (hasStyle(node, "rtl"))
		styleMap["direction"] = "rtl";
}

void Docx::addTableStyle(const pugi::xml_node& xmlNode) {
	m_borderMap.clear();
	auto styleNode = xmlNode.child("w:tblPr").child("w:tblBorders");
	for (const auto& border : BORDER_LIST) {
		std::string type  = getStyleValue(styleNode, "w:"+border, "w:val");
		std::string color = getStyleValue(styleNode, "w:"+border, "w:color");
		if (!type.empty()) {
			m_borderMap[border] = std::to_string(BORDER_SIZE.at(type)) +"px "+
								  BORDER_TYPE.at(type) +" #"+
								  ((color.empty() || color == "auto") ? "000" : color);
		}
	}
}

void Docx::addRowStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	auto styleNode     = xmlNode.child("w:trPr");
	std::string height = getStyleValue(styleNode, "w:trHeight", "w:val");
	if (!height.empty()) {
		height = "height:"+ std::to_string(stoi(height)/20) +"px";
		htmlNode.append_attribute("style") = height.c_str();
	}
}

void Docx::addCellStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) {
	std::unordered_map<std::string, std::string> styleMap;
	std::string value;
	auto styleNode = xmlNode.child("w:tcPr");

	value = getStyleValue(styleNode, "w:vAlign", "w:val");
	if (!value.empty())
		styleMap["vertical-align"] = VERT_ALIGN.at(value);

	value = getStyleValue(styleNode, "w:shd", "w:fill");
	if (!value.empty() && value != "auto")
		styleMap["background"] = "#"+ value;

	styleNode = styleNode.child("w:tcBorders");
	for (const auto& border : BORDER_LIST) {
		std::string type  = getStyleValue(styleNode, "w:"+border, "w:val");
		std::string color = getStyleValue(styleNode, "w:"+border, "w:color");
		if (!type.empty()) {
			styleMap["border-"+border] = std::to_string(BORDER_SIZE.at(type)) +"px "+
										 BORDER_TYPE.at(type) +" #"+
										 ((color.empty() || color == "auto") ? "000" : color);
		}
		else if (!m_borderMap[border].empty()) {
			styleMap["border-"+border] = m_borderMap[border];
		}
		else {
			styleMap["border-"+border] = "1px none #000";
		}
	}

	std::string style;
	for (const auto& sm : styleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (!style.empty())
		htmlNode.append_attribute("style") = style.c_str();
}

}  // End namespace