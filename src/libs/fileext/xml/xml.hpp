/**
 * @brief   XML files into HTML сonverter
 * @package xml
 * @file    xml.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0.1
 * @date    04.08.2016 -- 10.02.2018
 */
#pragma once

#include <string>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"


/**
 * @namespace xml
 * @brief
 *     XML files into HTML сonverter
 */
namespace xml {

/**
 * @class Xml
 * @brief
 *     XML files into HTML сonverter
 */
class Xml: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Xml(const std::string& fileName);

	/** Destructor */
	virtual ~Xml() = default;

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
	 *     Recursively walk through all XML-nodes
	 * @param[in] xmlNode
	 *     Current XML-node
	 * @param[out] htmlNode
	 *     Current HTML-node
	 * @since 1.0
	 */
	void treeWalker(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const;
};

}  // End namespace
