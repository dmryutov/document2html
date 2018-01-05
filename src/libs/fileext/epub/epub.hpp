/**
 * @brief     EPUB files into HTML сonverter
 * @package   epub
 * @file      epub.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @version   1.0
 * @date      14.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"
#include "../ooxml/ooxml.hpp"


/**
 * @namespace epub
 * @brief
 *     EPUB files into HTML сonverter
 */
namespace epub {

/**
 * @class Epub
 * @brief
 *     EPUB files into HTML сonverter
 */
class Epub: public fileext::FileExtension, public ooxml::Ooxml {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Epub(const std::string& fileName);

	/** Destructor */
	virtual ~Epub() = default;

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
	void convert(bool addStyle = true, bool extractImages = false, char mergingMode = 0) override;

private:
	/**
	 * @brief
	 *     Update links to files
	 * @since 1.0
	 */
	void updateLinks();

	/**
	 * @brief
	 *     Update links to images
	 * @since 1.0
	 */
	void updateImages();

	/** Used id of files */
	std::unordered_map<std::string, std::string> m_fileList;
};

}  // End namespace