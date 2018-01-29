/**
 * @brief     PPT files into HTML сonverter
 * @package   ppt
 * @file      ppt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.0
 * @date      05.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "../../pugixml/pugixml.hpp"
#include "../cfb/cfb.hpp"
#include "../fileext.hpp"


/**
 * @namespace ppt
 * @brief
 *     PPT files into HTML сonverter
 */
namespace ppt {

/**
 * @class Ppt
 * @brief
 *     PPT files into HTML сonverter
 */
class Ppt: public fileext::FileExtension, public cfb::Cfb {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Ppt(const std::string& fileName);

	/** Destructor */
	virtual ~Ppt() = default;

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
	 *     Get current structure length
	 * @param[in] stream
	 *     Stream data
	 * @param[in] offset
	 *     Structure offset
	 * @param[in] recType
	 *     Structure type
	 * @return
	 *     Structure length
	 * @since 1.0
	 */
	unsigned short getRecordLength(const std::string& stream, size_t offset,
									 unsigned short recType = 0) const;

	/**
	 * @brief
	 *     Get current structure type in accordance with MS documentation
	 * @param[in] stream
	 *     Stream data
	 * @param[in] offset
	 *     Structure offset
	 * @return
	 *     Structure type
	 * @since 1.0
	 */
	unsigned short getRecordType(const std::string& stream, int offset) const;

	/**
	 * @brief
	 *     Get record data from stream by offset, possibly a given type
	 * @note
	 *     Title is not sent back
	 * @param[in] stream
	 *     Stream data
	 * @param[in] offset
	 *     Structure offset
	 * @param[in] recType
	 *     Structure type
	 * @return
	 *     Record data
	 * @since 1.0
	 */
	std::string getRecord(const std::string& stream, size_t offset, unsigned short recType = 0) const;

	/**
	 * @brief
	 *     Add paragraph to HTML-tree
	 * @param[in] text
	 *     Paragraph text
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void addParagraph(const std::string& text, pugi::xml_node& htmlNode) const;
};

}  // End namespace
