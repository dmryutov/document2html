/**
 * @brief     TXT files into HTML сonverter
 * @package   txt
 * @file      txt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright adhocore (https://github.com/adhocore/htmlup)
 * @version   1.1
 * @date      01.08.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"


/**
 * @namespace txt
 * @brief
 *     TXT files into HTML сonverter
 */
namespace txt {

/**
 * @class Txt
 * @brief
 *     TXT files into HTML сonverter
 */
class Txt: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Txt(const std::string& fileName);

	/** Destructor */
	virtual ~Txt() = default;

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
	 *     Parse global elements
	 * @since 1.0
	 */
	void parseGlobalElements();

	/**
	 * @brief
	 *     Get images (extract local files or download from url)
	 * @since 1.1
	 */
	void getImages();

	/** HTML content */
	std::string m_html;
};

}  // End namespace