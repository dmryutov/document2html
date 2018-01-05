/**
 * @brief   JSON files into HTML сonverter
 * @package json
 * @file    json.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0
 * @date    04.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>

#include "../../json.hpp"
#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"


/**
 * @namespace json
 * @brief
 *     JSON files into HTML сonverter
 */
namespace json {

/**
 * @class Json
 * @brief
 *     JSON files into HTML сonverter
 */
class Json: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Json(const std::string& fileName);

	/** Destructor */
	virtual ~Json() = default;

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
	 *     Recursively walk through all JSON-objects
	 * @param[in] object
	 *     Current JSON-object
	 * @param[out] htmlNode
	 *     Current HTML-node
	 * @since 1.0
	 */
	void objectWalker(const nlohmann::json& object, pugi::xml_node& htmlNode) const;

	/**
	 * @brief
	 *     Add brackets before and after block
	 * @param[in] bracketOpen
	 *     Open bracket symbol
	 * @param[in] bracketClose
	 *     Close bracket symbol
	 * @param[in] object
	 *     Current JSON-object
	 * @param[out] htmlNode
	 *     Current HTML-node
	 * @param[in] shouldAddComma
	 *     True if should add comma to the end
	 * @since 1.0
	 */
	void addBrackets(const std::string& bracketOpen, const std::string& bracketClose,
					 const nlohmann::json& object, pugi::xml_node& htmlNode,
					 const bool shouldAddComma = false) const;

	/**
	 * @brief
	 *     Add comma to HTML-node
	 * @param[out] htmlNode
	 *     Current HTML-node
	 * @since 1.0
	 */
	void addComma(pugi::xml_node& htmlNode) const;
};

}  // End namespace