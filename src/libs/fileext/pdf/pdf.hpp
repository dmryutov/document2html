/**
 * @brief     PDF files into HTML сonverter
 * @package   pdf
 * @file      pdf.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.0
 * @date      06.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../pugixml/pugixml.hpp"
#include "../fileext.hpp"


/**
 * @namespace pdf
 * @brief
 *     PDF files into HTML сonverter
 */
namespace pdf {

/**
 * @class Pdf
 * @brief
 *     PDF files into HTML сonverter
 */
class Pdf: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Pdf(const std::string& fileName);

	/** Destructor */
	virtual ~Pdf() = default;

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
	 *     Get list of current object options
	 * @details
	 *     Options are between `<<` and `>>`. Each option starts with `/`
	 * @param[in] object
	 *     Current object
	 * @param[out] optionList
	 *     List of current object options
	 * @since 1.0
	 */
	void getObjectOptionList(const std::string& object,
							 std::unordered_map<std::string, std::string>& optionList) const;

	/**
	 * @brief
	 *     Decode stream data
	 * @details
	 *     There is a stream, possibly encoded by some compression method, or even several.
	 *     PDF supports a lot of methods, but text is encoded in three ways:
	 *     - ASCII Hex
	 *     - ASCII 85-base
	 *     - GZ/Deflate
	 * @param[in] stream
	 *     Stream data
	 * @param[in] optionList
	 *     List of current object options
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	std::string decodeStream(const std::string& stream,
							 std::unordered_map<std::string, std::string>& optionList) const;

	/**
	 * @brief
	 *     Decode ASCII Hex encoding method
	 * @param[in] input
	 *     Stream data
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	std::string decodeAsciiHex(const std::string& input) const;

	/**
	 * @brief
	 *     Decode ASCII 85-base encoding method
	 * @param[in] input
	 *     Stream data
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	std::string decodeAscii85(const std::string& input) const;

	/**
	 * @brief
	 *     Decode GZ/Deflate encoding method (the most common type of compression in PDF)
	 * @param[in] input
	 *     Stream data
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	std::string decodeFlate(const std::string& input) const;

	/**
	 * @brief
	 *     Decode CCITTFaxDecode encoding method (TIFF image format)
	 * @details
	 *     CCITTFaxDecode filter decodes image data that has been encoded using either Group 3 or
	 *     Group 4 CCITT facsimile (fax) encoding. CCITT encoding is designed to achieve
	 *     efficient compression of monochrome (1 bit per pixel) image data at relatively low
	 *     resolutions, and so is useful only for bitmap image data, not for color images,
	 *     grayscale images, or general data.
	 *     K possible values:
	 *     Value | Description
	 *     :---: | -----------
	 *     K < 0 | Pure two-dimensional encoding (Group 4)
	 *     K = 0 | Pure one-dimensional encoding (Group 3, 1-D)
	 *     K > 0 | Mixed one- and two-dimensional encoding (Group 3, 2-D)
	 * @param[in] input
	 *     Stream data
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	std::string decodeCcittFax(const std::string& input,
							   std::unordered_map<std::string, std::string>& optionList) const;

	/**
	 * @brief
	 *     Get array of dirty texts from `BT`-`ET` containers
	 * @param[in] stream
	 *     Stream data
	 * @return
	 *     True if at least one container contains dirty text
	 * @since 1.0
	 */
	bool getDirtyTextList(std::string stream);

	/**
	 * @brief
	 *     Get table of transformations of individual chars
	 * @details
	 *     Transformation is the translation of one character in hex-representation into another,
	 *     or even in some sequence. We are interested in following fields, which we need to find
	 *     in current stream. Data between `beginbfchar` and `endbfchar` converts one hex code to
	 *     another (or a sequence of codes) separately. Between `beginbfrange` and `endbfrange`,
	 *     transformation is organized over data sequences, which reduces number of definitions.
	 *
	 *     Firstly process individual characters. Conversion string looks like this:
	 *            Type       | Description
	 *     ----------------- | -----------
	 *     <0123> <abcd>     | `0123` is converted to `abcd`
	 *     <0123> <abcd6789> | `0123` is converted to several characters (`abcd` and `6789`)
	 *
	 *     Secondly process sequences. There are two types of sequence documentation:
	 *                       Type                   | Description
	 *     ---------------------------------------- | -----------
	 *     <0000> <0020> <0a00>                     | 0000 -> 0a00, 0001 -> 0a01 and so on to 0020
	 *     <0000> <0002> [<abcd> <01234567> <8900>] | Look, how much elements are between <0000> and
	 *                                                <0002> (together with 0001 three). Then each
	 *                                                of elements assign value from square brackets:
	 *                                                0000 -> abcd, 0001 -> 0123 4567, 0002 -> 8900
	 * @param[in] stream
	 *     Stream data
	 * @param[out] transformationList
	 *     Table of transformations
	 * @since 1.0
	 */
	void getTransformationList(const std::string& stream,
							   std::unordered_map<std::string, std::string>& transformationList) const;

	/**
	 * @brief
	 *     Extract images
	 * @param[in] imageData
	 *     Decoded image stream data
	 * @param[in] optionList
	 *     List of current object options
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @return
	 *     Decoded stream data
	 * @since 1.0
	 */
	void getImages(std::string imageData,
				   std::unordered_map<std::string, std::string>& optionList,
				   pugi::xml_node& htmlNode);

	/**
	 * @brief
	 *     Transform text and generate final text
	 * @details
	 *     Second stage - get text from "dirty" data. In PDF "dirty" text strings
	 *     can look like this:
	 *             Type        | Description
	 *     :-----------------: | -----------
	 *      (I love) 10 (C++)  | () contains text data, 10 - size of space
	 *          <01234567>     | 2 symbols in HEX-representation: 0123 and 4567
	 *     (Hello, \123world!) | \123 is the octal char code
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
	void transformText(pugi::xml_node& htmlNode) const;

	/**
	 * @brief
	 *     Write binary data
	 * @tparam T
	 *     Input data type (char/int/long)
	 * @param[in] data
	 *     Number value
	 * @param[in] size
	 *     Size of data chunk
	 * @param[in] isLittleEndian
	 *     Order of bytes. True if should reverse result string
	 * @return
	 *     Binary data
	 * @since 1.0
	 */
	template<typename T>
	std::string writeByte(const T& data, int size, bool isLittleEndian = false) const;

	/** Raw PDF data */
	std::string m_data;
	/** Array of dirty texts */
	std::vector<std::pair<std::string, std::string>> m_textList;
	/** Table of transformations */
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_transformationList;
	/** List of fonts and links to transformation table */
	std::unordered_map<std::string, std::pair<std::string, bool>> m_fontList;
	/** List of font names and links to objects */
	std::unordered_map<std::string, std::string> m_fontNameList;
};

}  // End namespace