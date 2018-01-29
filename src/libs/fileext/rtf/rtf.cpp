/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      rtf.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <unordered_map>

#include "formatting.hpp"
#include "keyword.hpp"
#include "table.hpp"

#include "rtf.hpp"


namespace rtf {

// public:
Rtf::Rtf(const std::string& fileName)
	: FileExtension(fileName) {}

void Rtf::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	std::string data;
	std::ifstream inputFile(m_fileName);
	data.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
	inputFile.close();

	bool hasAsterisk = false;
	std::vector<Formatting> formatStack;
	Formatting currentFormat;
	std::vector<Color> colorTable;
	std::unordered_map<int, Font> fontTable;
	std::vector<int> imageTagList;

	// CellDefs in rtf are really queer. Keep list of them in main() and
	// will give iterator into this list to row
	std::list<PtrVec<TableCellDef>> cellDefsList;
	std::list<PtrVec<TableCellDef>>::iterator curCellDefs;
	TableCellDef *tcdCurCellDef = new TableCellDef;
	TableCell *tcCurCell = new TableCell;
	TableRow  *trCurRow  = new TableRow;
	Table *tblCurTable   = new Table(m_mergingMode);
	int  lastRowLeft = 0;
	bool inTable     = false;
	// List variables
	pugi::xml_node lastLi;
	bool inList    = false;
	bool inLi      = false;
	int  listLevel = 0;

	m_nodeList.emplace_back(m_htmlTree.append_child("html").append_child("body"));
	HtmlText htmlText(currentFormat, m_addStyle);

	auto dataEnd = data.end();
	for (auto dataIter = data.begin(); dataIter != dataEnd; ) {
		auto node = m_nodeList.back();
		switch (*dataIter) {
			case '\\': {
				Keyword kw(++dataIter);
				if (kw.m_isControlChar) {
					switch (kw.m_controlChar) {
						case '\\': case '{': case '}':
							htmlText.add(kw.m_controlChar);
							break;
						case '\'':
							htmlText.add(codeToText(dataIter));
							break;
						case '*':
							// Get access to commented \pict block
							if (std::string(dataIter, dataIter + 8) != "\\shppict")
								hasAsterisk = true;
							break;
						case '~':
							htmlText.add("\u00A0");  // &nbsp;
							break;
					}
				}
				// Skip groups with \*
				else if (hasAsterisk) {
					hasAsterisk   = false;
					currentFormat = formatStack.back();
					formatStack.pop_back();
					skipGroup(dataIter);
				}
				else {
					std::string& keyword = kw.m_name;
					// Skip such groups
					if (keyword == "filetbl"  || keyword == "stylesheet" || keyword == "header"  ||
						keyword == "footer"   || keyword == "headerf"    || keyword == "footerf" ||
						keyword == "object"   || keyword == "info"       ||
						(keyword == "pict" && !m_extractImages)
					)
						skipGroup(dataIter);

					// Color table
					else if (keyword == "colortbl") {
						Color color;
						while (*dataIter != '}') {
							switch (*dataIter) {
								case '\\': {
									Keyword kwColor(++dataIter);
									if (kwColor.m_name == "red")
										color.m_red = kwColor.m_parameter;
									else if (kwColor.m_name == "green")
										color.m_green = kwColor.m_parameter;
									else if (kwColor.m_name == "blue")
										color.m_blue = kwColor.m_parameter;
									break;
								}
								case ';':
									colorTable.emplace_back(color);
									++dataIter;
									break;
								default:
									++dataIter;
									break;
							}
						}
						++dataIter;
					}

					// Font table
					else if (keyword == "fonttbl") {
						Font font;
						int fontNum;
						bool fullName = false;
						bool inFont   = false;
						while (!(*dataIter == '}' && !inFont)) {
							switch (*dataIter) {
								case '\\': {
									Keyword kwFont(++dataIter);
									if (kwFont.m_isControlChar && kwFont.m_controlChar == '*')
										skipGroup(dataIter);
									else if (kwFont.m_name == "f")
										fontNum = kwFont.m_parameter;
									else if (kwFont.m_name == "fcharset")
										font.m_charset = kwFont.m_parameter;
									else if (kwFont.m_name == "fnil")
										font.m_family = Font::FF_NONE;
									else if (kwFont.m_name == "froman")
										font.m_family = Font::FF_SERIF;
									else if (kwFont.m_name == "fswiss")
										font.m_family = Font::FF_SANS_SERIF;
									else if (kwFont.m_name == "fmodern")
										font.m_family = Font::FF_MONOSPACE;
									else if (kwFont.m_name == "fscript")
										font.m_family = Font::FF_CURSIVE;
									else if (kwFont.m_name == "fdecor")
										font.m_family = Font::FF_FANTASY;
									break;
								}
								case '{':
									inFont = true;
									++dataIter;
									break;
								case '}':
									inFont = false;
									fontTable[fontNum] = font;
									font = Font();
									fullName = false;
									++dataIter;
									break;
								case ';':
									fullName = true;
									++dataIter;
									break;
								default:
									if (!fullName && inFont)
										font.m_name += *dataIter;
									++dataIter;
									break;
							}
						}
						++dataIter;
						break;
					}

					// Pictures
					else if (keyword == "pict") {
						std::string hexData;
						std::string ext = "wmf";
						int bracketCount = 1;
						bool inTagList = false;
						int imageWidth, imageHeight;
						while (bracketCount > 0) {
							switch (*dataIter) {
								case '\\': {
									Keyword kwPict(++dataIter);
									if (kwPict.m_name == "emfblip")
										ext = "emf";
									else if (kwPict.m_name == "pngblip")
										ext = "png";
									else if (kwPict.m_name == "jpegblip")
										ext = "jpg";
									else if (kwPict.m_name == "macpict")
										ext = "pict";
									else if (kwPict.m_name == "bliptag") {
										int tag = kwPict.m_parameter;
										if (std::find(imageTagList.begin(), imageTagList.end(), tag) ==
											imageTagList.end())
										{
											imageTagList.emplace_back(tag);
										}
										else {
											inTagList = true;
										}
									}
									else if (kwPict.m_name == "picwgoal")
										imageWidth = 96 * kwPict.m_parameter / 1440;
									else if (kwPict.m_name == "pichgoal")
										imageHeight = 96 * kwPict.m_parameter / 1440;
									break;
								}
								case '{':
									bracketCount++;
									++dataIter;
									break;
								case '}':
									bracketCount--;
									++dataIter;
									break;
								case '\n':
								case '\r':
									++dataIter;
									break;
								default:
									if (bracketCount == 1 && !inTagList)
										hexData += *dataIter;
									++dataIter;
									break;
							}
						}

						if (!inTagList) {
							// Decode image from HEX representation
							std::string imageData;
							size_t hexDataSize = hexData.size();
							for (size_t i = 0; i < hexDataSize; i += 2) {
								char c = (char)std::stoi(hexData.substr(i, 2), nullptr, 16);
								imageData.push_back(c);
							}
							m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

							// Add `data-tag` attribute and some style
							std::string style = "width: " + std::to_string(imageWidth) + "px;";
							style += "height: " + std::to_string(imageHeight) + "px;";

							auto node = m_nodeList.back().append_child("p").append_child("img");
							node.append_attribute("data-tag") = m_imageList.size() - 1;
							node.append_attribute("style") = style.c_str();
						}
					}

					// Special chars
					else if (keyword == "line" || keyword == "softline")
						htmlText.add("\n");
					else if (keyword == "tab") {
						if (inLi)
							m_inBullet = false;
						else
							htmlText.add("\t");
					}
					else if (keyword == "enspace" || keyword == "emspace")
						htmlText.add("\u00A0");  // &nbsp;
					else if (keyword == "endash")
						htmlText.add("\u2013");  // &ndash;
					else if (keyword == "emdash")
						htmlText.add("\u2014");  // &mdash;
					else if (keyword == "bullet")
						htmlText.add("\u2022");  // &bull;
					else if (keyword == "lquote")
						htmlText.add("\u2018");  // &lsquo;
					else if (keyword == "rquote")
						htmlText.add("\u2019");  // &rsquo;
					else if (keyword == "ldblquote")
						htmlText.add("\u201C");  // &ldquo;
					else if (keyword == "rdblquote")
						htmlText.add("\u201D");  // &rdquo;

					// Paragraph formatting
					else if (keyword == "li")
						currentFormat.m_listLevel = kw.m_parameter/20;
					else if (keyword == "pard") {
						currentFormat.m_listLevel  = 0;
						currentFormat.m_parInTable = false;
					}
					else if (keyword == "par" || keyword == "sect") {
						htmlText.close();
						// Paragraph in tables
						if (inTable) {
							if (currentFormat.m_parInTable) {
								htmlText.addSubtree(tcCurCell->m_node);
								tcCurCell->m_node.append_child("br");
							}
							else {
								tblCurTable->make(node);
								// + t_str (= htmlText.str())
								inTable = false;
								delete tblCurTable;
								tblCurTable = new Table(m_mergingMode);
							}
						}
						else {
							// Lists
							if (inLi) {
								std::string parentName = lastLi.parent().name();
								std::string liName = m_isUl ? "ul" : "ol";
								bool changeList = (currentFormat.m_listLevel == listLevel &&
												   parentName != liName);

								// Increment list level
								if (currentFormat.m_listLevel > listLevel || changeList) {
									// Change list type (ul <-> ol)
									if (changeList) {
										lastLi = m_nodeList.back().parent();
										m_nodeList.pop_back();
									}

									node = lastLi.append_child(liName.c_str());
									m_nodeList.emplace_back(node);
									inList = true;
								}
								// Decrement list level
								if (currentFormat.m_listLevel < listLevel) {
									m_nodeList.pop_back();
									node = m_nodeList.back();
								}

								// Add list item
								node   = node.append_child("li");
								lastLi = node;

								listLevel = currentFormat.m_listLevel;
								inLi = false;
								m_isUl = false;
							}
							// Raw paragraphs
							else {
								// Close list tags
								if (inList) {
									inList = false;
									m_nodeList.pop_back();
									node = m_nodeList.back();
								}

								listLevel = currentFormat.m_listLevel;
								lastLi = node;
								node = node.append_child("p");
							}
							htmlText.addSubtree(node);
						}
						htmlText.clearText();
					}
					else if (keyword == "listtext") {
						inLi = true;
						m_inBullet = true;
					}
					else if (keyword == "pnlvlblt") {
						m_isUl = true;
					}

					// Character formatting
					else if (keyword == "b")
						currentFormat.m_isBold = !(kw.m_parameter == 0);
					else if (keyword == "i")
						currentFormat.m_isItalic = !(kw.m_parameter == 0);
					else if (keyword == "ul"     || keyword == "uldb"   || keyword == "ulth" ||
							 keyword == "ulw"    || keyword == "ulwave" || keyword == "uld"  ||
							 keyword == "uldash" || keyword == "uldashd"
					)
						currentFormat.m_isUnderlined = !(kw.m_parameter == 0);
					else if (keyword == "ulnone")
						currentFormat.m_isUnderlined = false;
					else if (keyword == "strike" || keyword == "striked")
						currentFormat.m_isStruckOut = !(kw.m_parameter == 0);
					else if (keyword == "outl")
						currentFormat.m_isOutlined = !(kw.m_parameter == 0);
					else if (keyword == "sub")
						currentFormat.m_isSub = !(kw.m_parameter == 0);
					else if (keyword == "super")
						currentFormat.m_isSup = !(kw.m_parameter == 0);

					else if (keyword == "fs")
						currentFormat.m_fontSize = kw.m_parameter / 2;
					else if (keyword == "f")
						currentFormat.m_font = fontTable[kw.m_parameter];

					else if (keyword == "cf")
						currentFormat.m_fontColor = colorTable[kw.m_parameter];
					else if (keyword == "cb" /* || keyword == "cbpat"*/)
						currentFormat.m_backgroundColor = colorTable[kw.m_parameter];

					// paragraph style
					else if (keyword == "ql")
						currentFormat.m_horizontalAlign = "left";
					else if (keyword == "qc")
						currentFormat.m_horizontalAlign = "center";
					else if (keyword == "qr")
						currentFormat.m_horizontalAlign = "right";
					else if (keyword == "qj")
						currentFormat.m_horizontalAlign = "justify";

					else if (keyword == "clvertalb")
						currentFormat.m_verticalAlign = "bottom";
					else if (keyword == "clvertalc")
						currentFormat.m_verticalAlign = "middle";
					else if (keyword == "clvertalt")
						currentFormat.m_verticalAlign = "top";

					else if (keyword == "plain") {
						currentFormat.m_isBold          = false;
						currentFormat.m_isItalic        = false;
						currentFormat.m_isUnderlined    = false;
						currentFormat.m_isStruckOut     = false;
						currentFormat.m_isOutlined      = false;
						currentFormat.m_isSub           = false;
						currentFormat.m_isSup           = false;
						currentFormat.m_fontSize        = 0;
						currentFormat.m_font            = Font();
						currentFormat.m_fontColor       = Color();
						currentFormat.m_backgroundColor = Color();
						currentFormat.m_horizontalAlign.clear();
						currentFormat.m_verticalAlign.clear();
					}

					// Table formatting
					else if (keyword == "intbl")
						currentFormat.m_parInTable = true;
					else if (keyword == "trowd")
						curCellDefs = cellDefsList.insert(cellDefsList.end(), PtrVec<TableCellDef>());
					else if (keyword == "row" || keyword == "nestrow") {
						if (!trCurRow->m_cellList.empty()) {
							trCurRow->m_cellDefList = curCellDefs;
							if (trCurRow->m_left == -1000)
								trCurRow->m_left = lastRowLeft;
							tblCurTable->push_back(trCurRow);
							trCurRow = new TableRow;
						}
						inTable = true;
					}
					else if (keyword == "cell") {
						htmlText.close();
						htmlText.addSubtree(tcCurCell->m_node);
						htmlText.clearText();

						trCurRow->m_cellList.push_back(tcCurCell);
						tcCurCell = new TableCell;
					}
					else if (keyword == "cellx") {
						tcdCurCellDef->m_right = kw.m_parameter;
						curCellDefs->push_back(tcdCurCellDef);
						tcdCurCellDef = new TableCellDef;
					}
					else if (keyword == "trleft") {
						trCurRow->m_left = kw.m_parameter;
						lastRowLeft      = kw.m_parameter;
					}
					else if (keyword == "clvmgf")
						tcdCurCellDef->m_isFirstMerged = true;
					else if (keyword == "clvmrg")
						tcdCurCellDef->m_isMerged = true;
				}
				break;
			}
			case '{':
				// Group opening actions
				formatStack.emplace_back(currentFormat);
				++dataIter;
				break;
			case '}':
				// Group closing actions
				currentFormat = formatStack.back();
				formatStack.pop_back();
				++dataIter;
				break;
			case 13: case 10:
				++dataIter;
				break;
			case '<':
				htmlText.add("\u003C");  // &lt;
				++dataIter;
				break;
			case '>':
				htmlText.add("\u003E");  // &gt;
				++dataIter;
				break;
			default:
				if (!m_inBullet)
					htmlText.add(*dataIter);
				++dataIter;
		}
	}

	delete tcCurCell;
	delete trCurRow;
	delete tblCurTable;
	delete tcdCurCellDef;
}


// private:
void Rtf::skipGroup(std::string::iterator& it) const {
	int count = 1;
	while (count) {
		switch (*it++) {
			case '{':
				count++;
				break;
			case '}':
				count--;
				break;
			case '\\': {
				Keyword kw(it);
				if (!kw.m_isControlChar && kw.m_name == "rkw_bin" && kw.m_parameter > 0)
					std::next(it, kw.m_parameter);
				break;
			}
		}
	}
}

std::string Rtf::codeToText(std::string::iterator& it) {
	// Decode char by hex code
	std::string stmp(1, *it++);
	stmp += *it++;
	if (m_inBullet) {
		if (stmp == "b7")
			m_isUl = true;
		return "";
	}

	// Decode \u0000 symbols
	auto br = (*(it-5) == '\r') ? it-6 : it-5;
	if (*(br-5) == '\\' && *(br-4) == 'u') {
		char code[5] = {*(br-3), *(br-2), *(br-1), *br, '\0'};
		std::string output;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string bytes = converter.to_bytes(wchar_t(atoi(code)));
		for (auto c : bytes) {
			int s = 0;
			std::stringstream ss;

			ss << 0 + (unsigned char)c;
			ss >> s;

			output.push_back(static_cast<unsigned char>(s));
		}
		return output;
	}

	// Decode \'00 symbols
	int code = strtol(stmp.c_str(), nullptr, 16);
	switch (code) {
		case 147:
			return "\u201C";  // &ldquo;
		case 148:
			return "\u201D";  // &rdquo;
		case 167:
			return "\u00A7";  // &sect;
		default:
			return std::string(1, (char)code);
	}
}

}  // End namespace
