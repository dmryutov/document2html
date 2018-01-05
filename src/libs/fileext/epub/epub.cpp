/**
 * @brief     EPUB files into HTML сonverter
 * @package   epub
 * @file      epub.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @date      14.08.2017 -- 18.10.2017
 */
#include <fstream>
#include <unordered_map>

#include "../../tools.hpp"

#include "epub.hpp"


namespace epub {

const std::string ID_PREFIX = "file-";

// public:
Epub::Epub(const std::string& fileName)
	: FileExtension(fileName) {}

void Epub::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	auto htmlTag = m_htmlTree.append_child("html");
	auto headTag = htmlTag.append_child("head");
	auto bodyTag = htmlTag.append_child("body");

	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "book.opf", tree);

	for (const auto& node : tree.child("package").child("manifest")) {
		std::string fileName = node.attribute("href").value();
		std::string fileId   = node.attribute("id").value();
		std::string fileType = node.attribute("media-type").value();
		// HTML content
		if (fileType == "application/xhtml+xml") {
			pugi::xml_document tree2;
			Ooxml::extractFile(m_fileName, fileName, tree2);

			auto fileDiv = bodyTag.append_child("div");
			fileDiv.append_attribute("id") = (ID_PREFIX + fileId).c_str();
			for (const auto& child : tree2.child("html").child("body"))
				fileDiv.append_copy(child);
		}
		// CSS styles
		else if (fileType == "text/css" && m_addStyle) {
			std::string style;
			Ooxml::extractFile(m_fileName, fileName, style);

			headTag.append_child("style").append_child(pugi::node_pcdata).set_value(style.c_str());
		}

		// Save id of files (new divs)
		m_fileList[fileName] = fileId;
	}

	updateLinks();
	updateImages();
}


// private:
void Epub::updateLinks() {
	for (auto& linkNode : m_htmlTree.select_nodes("//a")) {
		auto node = linkNode.node();
		std::string link = node.attribute("href").value();
		for (const auto& file : m_fileList) {
			std::string path = file.first;
			if (tools::endsWith(path, link)) {
				node.attribute("href").set_value(("#" + ID_PREFIX + m_fileList[path]).c_str());
				break;
			}
		}
	}
}

void Epub::updateImages() {
	if (!m_extractImages) {
		for (auto& imageNode : m_htmlTree.select_nodes("//img"))
			imageNode.node().parent().remove_child(imageNode.node());
		return;
	}

	int imageCount = 0;
	for (auto& imageNode : m_htmlTree.select_nodes("//img")) {
		auto node = imageNode.node();
		std::string link = node.attribute("src").value();
		for (const auto& file : m_fileList) {
			std::string path = file.first;
			if (tools::endsWith(path, link)) {
				// Load image
				std::string ext = path.substr(path.find_last_of('.') + 1);
				std::string imageData;
				Ooxml::extractFile(m_fileName, path, imageData);
				m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

				// Update attributes
				node.remove_attribute("src");
				node.append_attribute("data-tag") = imageCount;
				++imageCount;
				break;
			}
		}
	}
}

}  // End namespace