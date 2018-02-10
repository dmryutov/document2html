/**
 * @brief   Interface for file extensions
 * @package fileext
 * @file    fileext.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.1.1
 * @date    12.07.2016 -- 10.02.2018
 */
#pragma once

// Uncomment this line to enable downloading images from URL (requires `cUrl` library)
// #define DOWNLOAD_IMAGES

#include <string>
#include <vector>

#include "../pugixml/pugixml.hpp"


/**
 * @namespace fileext
 * @brief
 *     Interface for file extensions
 */
namespace fileext {

/**
 * @class FileExtension
 * @brief
 *     Base class for file extensions converters
 */
class FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	FileExtension(const std::string& fileName);

	/** Destructor */
	virtual ~FileExtension() = default;

	/**
	 * @brief
	 *     Convert file to HTML-tree
	 * @param[in] addStyle
	 *     Should read and add styles to HTML-tree
	 * @param[in] extractImages
	 *     True if should extract images
	 * @param[in] mergingMode
	 *     Colspan/rowspan processing mode
	 * @since 1.0
	 */
	virtual void convert(bool addStyle = true, bool extractImages = false, char mergingMode = 0) = 0;

	/**
	 * @brief
	 *     Save HTML-tree to file
	 * @param[in] dir
	 *     Name of result file directory
	 * @param[in] fileName
	 *     Name of result file
	 * @param[in] isConfig
	 *     If config mode
	 * @since 1.0
	 */
	void saveHtml(std::string dir, const std::string& fileName = "tmp.html") const;

	/**
	 * @brief
	 *     Load specific format style
	 * @param[in] node
	 *     HTML `head` element
	 * @param[in] style
	 *     Inline style
	 * @since 1.1.1
	 */
	void loadStyle(pugi::xml_node& node, const std::string& style) const;

	/** Result HTML tree */
	pugi::xml_document m_htmlTree;

protected:
	/** Name of processing file */
	const std::string m_fileName;
	/** Should read and add styles to HTML-tree */
	bool m_addStyle = true;
	/**
	 * Colspan/rowspan processing mode
	 * Value | Description
	 * :---: | -----------
	 *   0   | Cells are merged
	 *   1   | Delete merging, fill cells with duplicate values
	 *   2   | Delete merging, fill cells with empty values
	 */
	char m_mergingMode = 0;
	/** True if should extract images */
	bool m_extractImages = false;
	/** List of images (binary data and extension) */
	std::vector<std::pair<std::string, std::string>> m_imageList;
};

}  // End namespace
