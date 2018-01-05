/**
 * @brief     TXT files into HTML сonverter
 * @package   txt
 * @file      txt.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright adhocore (https://github.com/adhocore/htmlup)
 * @date      01.08.2016 -- 18.10.2017
 */
#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#include "../../tools.hpp"

#include "txt.hpp"

#ifdef DOWNLOAD_IMAGES
#	include "../../curlwrapper/curlwrapper.hpp"
#endif


namespace txt {

/** List symbol list */
const std::vector<std::string> LIST_SYMBOL {"- ", "* ", "+ "};
const std::regex URL_REGEX("<(https?:[\\/]{2}[^\\s]+?)>", std::regex::icase);
const std::regex EMAIL_REGEX("<(\\S+?@\\S+?)>");
const std::regex IMAGE_REGEX("!\\[(.+?)\\]\\s*\\((.+?)\\s*(\"(.+?)\")?\\)");
const std::regex ANCHOR_REGEX("\\[(.+?)\\]\\s*\\((.+?)\\s*(\"(.+?)\")?\\)");
const std::regex BOLD_REGEX("(\\*{2}|_{2})(.+?)\\1");
const std::regex ITALIC_REGEX("(\\*|_)(.+?)\\1");
const std::regex STRIKE_REGEX("(~~)(.+?)\\1");
const std::regex HTML_REGEX("^<\\/?\\w.*?\\/?>.*");
const std::regex BLOCKQUOTE_REGEX("^(\\s*(>+)\\s+).*");
const std::regex HEADER_REGEX("^\\s*(={3,}|-{3,})\\s*$");
const std::regex RULE_REGEX("^(_{3,}|\\*{3,}|\\-{3,})$");
const std::regex LIST_REGEX("^\\d+\\. .*");
const std::regex TABLE_REGEX("(\\|\\s*\\:)?\\s*\\-{3,}\\s*(\\:\\s*\\|)?");

// public:
Txt::Txt(const std::string& fileName)
	: FileExtension(fileName) {}

void Txt::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	std::vector<std::string> data;
	std::string line;

	std::ifstream inputFile(m_fileName);
	while (getline(inputFile, line))
		data.emplace_back(tools::trim(line, "\r\n"));
	inputFile.close();

	// If style flag = `false`
	if (!m_addStyle) {
		auto mainNode = m_htmlTree.append_child("html").append_child("body");
		for (const auto& line : data)
			mainNode.append_child("p").append_child(pugi::node_pcdata).set_value(line.c_str());
		return;
	}

	// If style flag = `true`
	std::vector<std::string> stackList, stackBlock, stackTable;
	int  nestLevel  = 0;
	int  quoteLevel = 0;
	bool inList     = false;
	bool inQuote    = false;
	bool inPara     = false;
	bool inHtml     = false;
	bool inTable    = false;
	auto dataBegin  = data.begin();
	auto dataEnd    = data.end();

	m_html += "<html><body>";
	for (auto dataIter = dataBegin; dataIter != dataEnd; ++dataIter) {
		line = *dataIter;
		std::string trimmedLine = tools::trim(line);

		// Flush stacks at the end of block
		if (trimmedLine.empty()) {
			while (!stackList.empty()) {
				m_html += stackList.back();
				stackList.pop_back();
			}
			while (!stackBlock.empty()) {
				m_html += stackBlock.back();
				stackBlock.pop_back();
			}
			while (!stackTable.empty()) {
				m_html += stackTable.back();
				stackTable.pop_back();
			}
			m_html += "\n";

			inQuote    = false;
			inList     = false;
			inPara     = false;
			inHtml     = false;
			nestLevel  = 0;
			quoteLevel = 0;
			continue;
		}

		// Raw HTML
		if (std::regex_match(trimmedLine, HTML_REGEX) || inHtml) {
			m_html += "\n" + line;
			if (!inHtml && std::prev(dataIter)->empty())
				inHtml = true;
			continue;
		}

		std::string nextLine        = (std::next(dataIter) == dataEnd) ? "" : *std::next(dataIter);
		std::string trimmedNextLine = tools::trim(nextLine);
		std::string nextMark12      = trimmedNextLine.empty() ? "" : trimmedNextLine.substr(0, 2);

		int indent     = line.size() - tools::ltrim(line).size();
		int nextIndent = nextLine.empty() ? 0 : nextLine.size() - tools::ltrim(nextLine).size();

		// Blockquote
		std::smatch quoteMatch;
		if (regex_match(line, quoteMatch, BLOCKQUOTE_REGEX)) {
			line        = line.substr(quoteMatch[1].str().size());
			trimmedLine = tools::trim(line);
			if (!inQuote || quoteLevel < static_cast<int>(quoteMatch[2].str().size())) {
				m_html += "\n<blockquote>";
				stackBlock.emplace_back("\n</blockquote>");
				++quoteLevel;
			}
			inQuote = true;
		}

		char mark1         = trimmedLine[0];
		std::string mark12 = trimmedLine.substr(0, 2);

		// `H1`-`H6` tags
		if (mark1 == '#') {
			int level = trimmedLine.size() - tools::ltrim(trimmedLine, "#").size();
			if (level < 7) {
				m_html += "\n<h" + std::to_string(level) +">"+ tools::ltrim(trimmedLine, "# ") +
						"</h"+ std::to_string(level) +">";
				continue;
			}
		}

		// Alternative form of `H1`, `H2` tags
		if (regex_match(nextLine, HEADER_REGEX)) {
			int level = tools::trim(nextLine, "- ").empty() ? 2 : 1;
			m_html += "\n<h" + std::to_string(level) +">"+ trimmedLine +
					"</h"+ std::to_string(level) +">";
			++dataIter;
			continue;
		}

		// HR
		if (dataIter != dataBegin &&
			tools::trim(*std::prev(dataIter)).empty() &&
			regex_match(trimmedLine, RULE_REGEX)
		) {
			m_html += "\n<hr />";
			continue;
		}

		// List
		bool ul = find(LIST_SYMBOL.begin(), LIST_SYMBOL.end(), mark12) != LIST_SYMBOL.end();
		if (ul || regex_match(trimmedLine, LIST_REGEX)) {
			if (!inList) {
				std::string wrapper = ul ? "ul" : "ol";
				m_html += "\n<"+ wrapper +">\n";
				stackList.emplace_back("</"+ wrapper +">");
				inList = true;
				++nestLevel;
			}

			m_html += "<li>"+ tools::ltrim(trimmedLine, "-*0123456789. ");

			ul = find(LIST_SYMBOL.begin(), LIST_SYMBOL.end(), nextMark12) != LIST_SYMBOL.end();
			if (ul || regex_match(trimmedNextLine, LIST_REGEX)) {
				if (nextIndent > indent) {
					std::string wrapper = ul ? "ul" : "ol";
					m_html += "\n<"+ wrapper +">\n";
					stackList.emplace_back("</li>\n");
					stackList.emplace_back("</"+ wrapper +">");
					++nestLevel;
				}
				else {
					m_html += "</li>\n";
				}

				// Handle nested lists ending
				if (nextIndent < indent) {
					int shift = (indent - nextIndent) / 4;
					while (shift--) {
						m_html += stackList.back();
						stackList.pop_back();
						if (nestLevel > 2) {
							m_html += stackList.back();
							stackList.pop_back();
						}
					}
				}
			}
			else {
				m_html += "</li>";
			}

			continue;
		}

		if (inList) {
			m_html += trimmedLine;
			continue;
		}

		// Table
		std::string cell = tools::trim(trimmedLine, "|");
		int headerCount  = count(cell.begin(), cell.end(), '|');
		cell = tools::trim(trimmedNextLine, "|");

		if (!inTable) {
			int colCount = distance(
								std::sregex_iterator(cell.begin(), cell.end(), TABLE_REGEX),
								std::sregex_iterator()
							);

			if (headerCount != 0 && headerCount <= colCount) {
				inTable = true;
				++dataIter;
				m_html += "<table><thead><tr>\n";

				std::stringstream row(tools::trim(trimmedLine, "|"));
				std::string th;
				while (getline(row, th, '|'))
					m_html += "<th>"+ tools::trim(th) +"</th>\n";
				m_html += "</tr></thead><tbody>\n";
				continue;
			}
		}
		else {
			m_html += "<tr>\n";

			std::stringstream row(tools::trim(trimmedLine, "|"));
			std::string td;
			int i = 0;
			while (getline(row, td, '|') && i++ <= headerCount)
				m_html += "<td>"+ tools::trim(td) +"</td>";

			m_html += "</tr>\n";
			std::string cell = tools::trim(trimmedNextLine, "|");
			if (trimmedNextLine.empty() || !count(cell.begin(), cell.end(), '|')) {
				inTable = false;
				stackTable.emplace_back("</tbody></table>\n");
			}

			continue;
		}

		// Paragraph
		if (!inPara)
			m_html += "\n<p>";
		else
			m_html += "\n<br />";
		m_html += trimmedLine;

		if (trimmedNextLine.empty()) {
			m_html += "</p>";
			inPara = false;
		}
		else {
			inPara = true;
		}
	}
	m_html += "</body></html>";

	parseGlobalElements();

	m_htmlTree.load_string(m_html.data());
	if (m_extractImages)
		getImages();
}


// private:
void Txt::parseGlobalElements() {
	// URL
	m_html = regex_replace(m_html, URL_REGEX, "<a href=\"$1\">$1</a>");
	// Email
	m_html = regex_replace(m_html, EMAIL_REGEX, "<a href=\"mailto:$1\">$1</a>");
	// Image
	m_html = regex_replace(m_html, IMAGE_REGEX, "<img src=\"$2\" title=\"$4\" alt=\"$1\" />");
	// Anchor
	m_html = regex_replace(m_html, ANCHOR_REGEX, "<a href=\"$2\" title=\"$4\" >$1</a>");
	// Bold/Italic/Strike out
	m_html = regex_replace(m_html, BOLD_REGEX, "<b>$2</b>");
	m_html = regex_replace(m_html, ITALIC_REGEX, "<i>$2</i>");
	m_html = regex_replace(m_html, STRIKE_REGEX, "<s>$2</s>");

	// Join adjacent `b`, `i`, `s` tags
	tools::replaceAll(m_html, "</b><b>", "");
	tools::replaceAll(m_html, "</i><i>", "");
	tools::replaceAll(m_html, "</s><s>", "");
}

void Txt::getImages() {
	size_t pos = m_fileName.find_last_of('/');
	std::string dir = (pos == std::string::npos) ? "" : m_fileName.substr(0, pos);

	for (auto& imageNode : m_htmlTree.select_nodes("//img")) {
		auto node = imageNode.node();
		std::string link = node.attribute("src").value();
		if (link[0] == '/') link = link.substr(1);
		std::string path = dir + link;

		std::string imageData;
		// Try to find image on disk
		if (tools::fileExists(path)) {
			std::ifstream image(path, std::ios::binary);
			imageData.assign(std::istreambuf_iterator<char>(image), std::istreambuf_iterator<char>());
		}
		// Try to download img from URL
		else {
			#ifdef DOWNLOAD_IMAGES
				imageData = curlwrapper::downloadFile(link);
			#endif
		}

		if (!imageData.empty()) {
			std::string ext = link.substr(link.find_last_of('.') + 1);
			m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

			// Update attributes
			node.remove_attribute("src");
			node.append_attribute("data-tag") = m_imageList.size() - 1;
		}
	}
}

}  // End namespace
