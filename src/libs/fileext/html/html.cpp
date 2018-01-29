/**
 * @brief   Wrapper for HTML files
 * @package html
 * @file    html.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    04.08.2016 -- 28.01.2018
 */
#include <algorithm>
#include <fstream>
#include <regex>

#include "../../encoding/encoding.hpp"
#include "../../tools.hpp"

#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
	#include <tidybuffio.h>
	#include <tidy.h>
#else
	#include <tidy/buffio.h>
	#include <tidy/tidy.h>
#endif

#include "html.hpp"

#ifdef DOWNLOAD_IMAGES
#	include "../../curlwrapper/curlwrapper.hpp"
#endif


namespace html {

const std::regex ENCODING_MASK(" charset=(\"|)(.*?)(\"| )", std::regex::icase);

// public:
Html::Html(const std::string& fileName)
	: FileExtension(fileName) {}

void Html::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	// Open file
	std::ifstream inputFile(m_fileName);
	m_data.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
	inputFile.close();

	// Ensure `html`, `head`, `body` tags
	/*if (!imatch(m_data, "<html>"))
		m_data = "<html>" + m_data;
	if (!imatch(m_data, "</html>"))
		m_data += "</html>";
	if (!imatch(m_data, "<head>"))
		ireplaceAll(m_data, "<html>", "<html><head></head>");
	if (!imatch(m_data, "<body>"))
		ireplaceAll(m_data, "</head>", "</head><body>");
	if (!imatch(m_data, "</body>"))
		ireplaceAll(m_data, "</html>", "</body></html>");*/
	ireplaceAll(m_data, "<tbody>",  "");
	ireplaceAll(m_data, "</tbody>", "");
	ireplaceAll(m_data, "<thead>",  "");
	ireplaceAll(m_data, "</thead>", "");
	ireplaceAll(m_data, "<nobr>", "");
	ireplaceAll(m_data, "</nobr>", "");
	ireplaceAll(m_data, "&#8226;",  "");
	ireplaceAll(m_data, "&bull;",   "");

	// Cut UTF-8 BOM first 3 bytes
	if (m_data[0] == '\xef' && m_data[1] == '\xbb' && m_data[2] == '\xbf')
		m_data = m_data.substr(3);

	// Tidy options
	TidyDoc tidyDoc = tidyCreate();
	TidyBuffer tidyOutputBuffer = {0, 0, 0, 0, 0};
	tidyOptSetInt(tidyDoc,   TidyDoctypeMode, TidyDoctypeOmit);
	tidyOptSetInt(tidyDoc,   TidyShowErrors, 0);
	tidyOptSetBool(tidyDoc,  TidyShowWarnings, no);
	tidyOptSetBool(tidyDoc,  TidyQuiet, yes);
	tidyOptSetBool(tidyDoc,  TidyXmlOut, yes);
	tidyOptSetBool(tidyDoc,  TidyEscapeCdata, yes);
	tidyOptSetBool(tidyDoc,  TidyHideComments, yes);
	tidyOptSetBool(tidyDoc,  TidyFixUri, yes);
	tidyOptSetBool(tidyDoc,  TidyLiteralAttribs, yes);
	//tidyOptSetBool(tidyDoc,  TidyMetaCharset, yes);
	tidyOptSetValue(tidyDoc, TidyCharEncoding, "raw");
	tidyOptSetValue(tidyDoc, TidyPreTags, "noindex");
	tidyOptSetValue(tidyDoc, TidyBlockTags, "article,aside,command,canvas,dialog,details,"
											"figcaption,figure,footer,header,main,hgroup,menu,"
											"nav,section,summary,meter,irblock");
	tidyOptSetValue(tidyDoc, TidyInlineTags, "video,audio,canvas,ruby,rt,rp,time,meter,progress,"
											 "datalist,keygen,mark,output,source,wbr,nobr");
	// Repair markup
	tidyParseString(tidyDoc, m_data.c_str());
	tidyCleanAndRepair(tidyDoc);
	tidySaveBuffer(tidyDoc, &tidyOutputBuffer);
	if (tidyOutputBuffer.bp) {
		std::string page = (char*)tidyOutputBuffer.bp;
		tidyBufFree(&tidyOutputBuffer);
		convertEncoding(page);
		m_htmlTree.load_buffer(page.c_str(), page.size());
	}
	else {
		m_htmlTree.load_string(m_data.c_str());
	}
	tidyRelease(tidyDoc);

	// Clear `meta` tags
	auto node = m_htmlTree.child("html");
	if (node.child("head")) {
		node.child("head").remove_child("meta");
		if (!m_addStyle)
			node.remove_child("head");
	}

	if (m_extractImages)
		getImages();
	deleteMerging();
}


// private:
void Html::ireplaceAll(std::string& str, const std::string& from, const std::string& to) const {
	if (from.empty())
		return;
	size_t pos = 0;
	size_t fromSize = from.size();
	while ((pos = ifind(str, from, static_cast<int>(pos))) != std::string::npos) {
		str.replace(pos, fromSize, to);
		// If `to` contains `from` (like replacing 'x' with 'yx')
		pos += to.size();
	}
}

bool Html::imatch(const std::string& haystack, const std::string& needle, int start) const {
	auto it = search(haystack.begin() + start, haystack.end(), needle.begin(), needle.end(),
		[](char ch1, char ch2) {
			return tolower(ch1) == tolower(ch2);
		}
	);
	return (it != haystack.end());
}

size_t Html::ifind(const std::string& haystack, const std::string& needle, int start) const {
	auto it = search(haystack.begin() + start, haystack.end(), needle.begin(), needle.end(),
		[](char ch1, char ch2) {
			return tolower(ch1) == tolower(ch2);
		}
	);
	if (it == haystack.end())
		return std::string::npos;
	else
		return distance(haystack.begin(), it);
}

void Html::deleteMerging() const {
	if (m_mergingMode == 0)
		return;

	for (auto& table : m_htmlTree.select_nodes(".//table")) {
		for (auto tr = table.node().first_child(); tr; tr = tr.next_sibling()) {
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
}

void Html::getImages() {
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

void Html::convertEncoding(std::string& page) const {
	std::smatch match;
	if (std::regex_search(page, match, ENCODING_MASK)) {
		std::string meta = tools::ltrim(match[2]);
		std::transform(meta.begin(), meta.end(), meta.begin(), ::toupper);

		if (meta.empty() || meta == "UTF-8")
			return;
		page = encoding::decode(page, meta);
	}
}

}  // End namespace
