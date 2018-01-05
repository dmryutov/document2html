/**
 * @brief   Wrapper for HTML files
 * @package html
 * @file    html.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 2.0
 * @date    04.08.2016 -- 11.11.2017
 */
#pragma once

#include <string>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"


/**
 * @namespace html
 * @brief
 *     Wrapper for HTML files
 */
namespace html {

/**
 * @class Html
 * @brief
 *     Wrapper for HTML files
 */
class Html: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Html(const std::string& fileName);

	/** Destructor */
	virtual ~Html() = default;

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
	 *     Case insensitive replacement of all occurrences of substring
	 * @param[in,out] str
	 *     Input string
	 * @param[in] from
	 *     String to search for
	 * @param[in] to
	 *     String to replace the search string with
	 * @since 1.0
	 */
	void ireplaceAll(std::string& str, const std::string& from, const std::string& to) const;

	/**
	 * @brief
	 *     Case insensitive matching of substring
	 * @param[in] haystack
	 *     Input string
	 * @param[in] needle
	 *     The value that you want to match
	 * @param[in] start
	 *     Search start position
	 * @return
	 *     True if needle is in input string
	 * @since 1.0
	 */
	bool imatch(const std::string& haystack, const std::string& needle, int start = 0) const;

	/**
	 * @brief
	 *     Case insensitive searching of position of first occurrence of substring
	 * @param[in] haystack
	 *     Input string
	 * @param[in] needle
	 *     The value that you want to match
	 * @param[in] start
	 *     Search start position
	 * @return
	 *     Position of needle in input instring
	 * @since 1.0
	 */
	size_t ifind(const std::string& haystack, const std::string& needle, int start = 0) const;

	/**
	 * @brief
	 *     Delete colspans and rowspans in tables
	 * @since 1.0
	 */
	void deleteMerging() const;

	/**
	 * @brief
	 *     Get images (extract local files or download from url)
	 * @since 1.1
	 */
	void getImages();

	/**
	 * @brief
	 *     Convert page encoding to UTF-8
	 * @param[in,out] page
	 *     Page content
	 * @since 2.0
	 */
	void convertEncoding(std::string& page) const;

	/** Raw HTML data */
	std::string m_data;
};

}  // End namespace
