/**
 * @brief     ODT files into HTML сonverter
 * @package   odt
 * @file      odt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @version   1.0
 * @date      12.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"
#include "../ooxml/ooxml.hpp"


/**
 * @namespace odt
 * @brief
 *     ODT files into HTML сonverter
 */
namespace odt {

/**
 * @class Odt
 * @brief
 *     ODT files into HTML сonverter
 */
class Odt: public fileext::FileExtension, public ooxml::Ooxml {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Odt(const std::string& fileName);

	/** Destructor */
	virtual ~Odt() = default;

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
	/// @name Loading styles
	/// @{
	/**
	 * @brief
	 *     Get style from `content.xml`
	 * @param[in] tree
	 *     XML document content
	 * @since 1.0
	 */
	void getStyleMap(const pugi::xml_document& tree);

	/**
	 * @brief
	 *     Get list style from `styles.xml`
	 * @since 1.0
	 */
	void getListStyleMap();
	/// @}

	/// @name Building elements
	/// @{

	/**
	 * @brief
	 *     Build element
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @param[in] inElement
	 *     True if parent node is `li`, `td`, etc
	 * @since 1.0
	 */
	void buildElement(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode, bool inElement);

	/**
	 * @brief
	 *     Create paragraph
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildParagraph(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Get plain text and add it to HTML tag
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildPlainText(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Create hyperlink
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildHyperlink(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Create image
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildImage(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Create table with all rows and cells correctly populated
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildTable(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Create single tr element, with all tds already populated
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildTr(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Delete cell merging (colspan/rowspan)
	 * @param[in] table
	 *     HTML table node
	 * @since 1.0
	 */
	void deleteMerging(pugi::xml_node& table) const;

	/**
	 * @brief
	 *     Build list structure
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void buildList(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Get list indentation level
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[in,out] node
	 *     Node id (key in `m_styleMap`)
	 * @return
	 *     Indentation level
	 * @since 1.0
	 */
	int getIndentationLevel(pugi::xml_node xmlNode, std::string& key) const;
	/// @}

	/// @name Style
	/// @{
	/**
	 * @brief
	 *     Add text style tags such as b/i/u/...
	 * @param[in] node
	 *     XML-node
	 * @param[in] style
	 *     Style tag name
	 * @since 1.0
	 */
	void addTextStyle(pugi::xml_node& node, const std::string& style) const;

	/**
	 * @brief
	 *     Add style to element
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void addStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Add general parent and element style
	 * @param[in] node
	 *     XML-node
	 * @param[out] styleMap
	 *     Reference to style map
	 * @since 1.0
	 */
	void addGeneralStyle(const std::string& key,
						 std::unordered_map<std::string, std::string>& styleMap);

	/**
	 * @brief
	 *     Add style to image
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void addImageStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const;

	/**
	 * @brief
	 *     Add style to table element
	 * @param[in] xmlNode
	 *     XML-node
	 * @since 1.0
	 */
	void addTableStyle(const pugi::xml_node& xmlNode);

	/**
	 * @brief
	 *     Add style to table row element
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void addRowStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Add style to table cell element
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void addCellStyle(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode);
	/// @}

	/** Document style information */
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_styleMap;
	/** List style information */
	std::unordered_map<std::string, std::unordered_map<int, std::unordered_map<std::string, std::string>>> m_listStyleMap;
	/** Stores table border style */
	std::unordered_map<std::string, std::string> m_borderMap;
};

}  // End namespace