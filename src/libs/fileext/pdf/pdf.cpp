/**
 * @brief     PDF files into HTML сonverter
 * @package   pdf
 * @file      pdf.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @date      06.08.2017 -- 18.10.2017
 */
#include <math.h>
#include <fstream>
#include <regex>

#include "../../encoding/encoding.hpp"
#include "../../lodepng/lodepng.h"
#include "../../miniz/miniz.h"
#include "../../tools.hpp"

#include "pdf.hpp"


namespace pdf {

/** Hex string char list */
const std::regex OBJECT_MASK("([0-9]+\\s*[0-9]+\\s*)obj((.|\n|\r)*?)endobj", std::regex::icase);
const std::regex STREAM_MASK("stream((.|\n|\r)*?)endstream", std::regex::icase);
const std::regex OPTIONS_MASK("<<((.|\n|\r)*?)>>", std::regex::icase);
const std::regex BT_ET_MASK("BT((.|\n|\r)*?)ET", std::regex::icase);
const std::regex TJ_FONT_MASK("\\/F[a-z]*([0-9]+)((.|\n|\r)*?)\\[((.|\n|\r)*?)\\]\\s*TJ", std::regex::icase);
const std::regex TJ_MASK("\\[((.|\n|\r)*?)\\]\\s*TJ", std::regex::icase);
const std::regex TD_TJ_MASK("Td\\s*\\/F[a-z]*([0-9]+)((.|\n|\r)*?)(\\((.|\n|\r)*?\\))\\s*Tj");
const std::regex CHAR_MASK("([0-9]+)\\s+beginbfchar((.|\n|\r)*?)endbfchar", std::regex::icase);
const std::regex CHAR_TYPE1_MASK("<([0-9a-fA-F]{2,4})>\\s+<([0-9a-fA-F]{4,512})>", std::regex::icase);
const std::regex RANGE_MASK("([0-9]+)\\s+beginbfrange((.|\n|\r)*?)endbfrange", std::regex::icase);
const std::regex RANGE_TYPE1_MASK("<([0-9a-fA-F]{1,4})>\\s*<([0-9a-fA-F]{1,4})>\\s*<([0-9a-fA-F]{1,4})>", std::regex::icase);
const std::regex RANGE_TYPE2_MASK("<([0-9a-fA-F]{1,4})>\\s*<([0-9a-fA-F]{1,4})>\\s*\\[(.*?)\\]", std::regex::icase);
const std::regex SPEC_CHAR_MASK("\\s+");
const std::regex TEXT_CHAR_MASK("[^0-9]");

// public:
Pdf::Pdf(const std::string& fileName)
	: FileExtension(fileName) {}

void Pdf::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;
	auto mainNode   = m_htmlTree.append_child("html").append_child("body");

	std::ifstream inputFile(m_fileName, std::ios::binary);
	m_data.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
	inputFile.close();

	if (m_data.empty())
		return;

	// First stage. Get all dirty text data from file - text, positioning, hex-data, etc
	// Get list of all objects (text, fonts, etc)
	std::smatch objectList;

	while (std::regex_search(m_data, objectList, OBJECT_MASK)) {
		std::string currentObject = objectList[2];

		// Get object options. We need only text data, so do minimal clipping to speed up
		std::unordered_map<std::string, std::string> optionList;
		getObjectOptionList(currentObject, optionList);
		optionList["OBJECT_ID"] = std::string(objectList[1]) + "R";

		// Check if curent object has stream (almost always it is compressed with gzip)
		std::smatch streamList;
		if (std::regex_search(currentObject, streamList, STREAM_MASK)) {
			std::string stream = tools::ltrim(streamList[1]);
			std::string streamData = decodeStream(stream, optionList);

			if (optionList.find("Image") != optionList.end()) {
				getImages(streamData, optionList, mainNode);
			}
			// So, there is "possible" text. Decode it from binary representation.
			// After this => dealing only with plain text
			else if (optionList.find("Length1") == optionList.end() &&
					 optionList.find("Type")    == optionList.end() &&
					 optionList.find("Subtype") == optionList.end() &&
					 !streamData.empty())
			{
				// Should find container of text in current stream. If successful, founded "dirty"
				// text will be added to the others founded before. Otherwise, try to find char
				// transformations that will be used in the second step
				bool hasDirtyText = getDirtyTextList(streamData);
				if (!hasDirtyText) {
					std::unordered_map<std::string, std::string> tList;
					getTransformationList(streamData, tList);
					m_transformationList[optionList["OBJECT_ID"]] = tList;
				}
			}
		}
		else if (optionList.find("Font") != optionList.end() &&
				 optionList.find("Type") != optionList.end())
		{
			bool isMultiByte = (optionList.find("Identity-H") != optionList.end() ||
								optionList.find("WinAnsiEncoding") != optionList.end());
			std::string table = (optionList.find("ToUnicode") != optionList.end())
								? optionList["ToUnicode"] : "";
			m_fontList[optionList["OBJECT_ID"]] = {table, isMultiByte};
		}
		else if (optionList.find("F1") != optionList.end() &&
				 optionList.find("Name") == optionList.end())
		{
			int intCounter = 1;
			std::string strCounter = "1";
			while (optionList.find(std::string("F") + strCounter) != optionList.end()) {
				m_fontNameList[strCounter] = optionList[std::string("F") + strCounter];
				intCounter++;
				strCounter = std::to_string(intCounter);
			}
		}
		m_data = objectList.suffix().str();
	}

	// At the end of the first iteration, start analysis of received text blocks
	// (taking into account char transformations)
	transformText(mainNode);
}


// private:
void Pdf::getObjectOptionList(const std::string& object,
							  std::unordered_map<std::string, std::string>& optionList) const
{
	// Options are between `<<` and `>>`. Each option starts with `/`
	std::smatch opts;
	if (std::regex_search(object, opts, OPTIONS_MASK)) {
		// Separate options by `/`
		std::vector<std::string> tempOptionList = tools::explode(opts[1], '/');
		// Options like `/ Option N` will be added to dictionary as `Option` => `N`,
		// options like `/ Param` as `Param` => `true`
		for (size_t i = 1; i < tempOptionList.size(); ++i) {
			tempOptionList[i] = regex_replace(tools::trim(tempOptionList[i]), SPEC_CHAR_MASK, " ");
			size_t pos = tempOptionList[i].find(' ');
			if (pos != std::string::npos)
				optionList[tempOptionList[i].substr(0, pos)] = tempOptionList[i].substr(pos + 1);
			else
				optionList[tempOptionList[i]] = "true";
		}
	}
}

std::string Pdf::decodeStream(const std::string& stream,
							  std::unordered_map<std::string, std::string>& optionList) const
{
	std::string data;

	// If current stream has `Filter` option then it is exactly compressed or encrypted
	if (optionList.find("Filter") == optionList.end()) {
		data = stream;
	}
	else {
		// If current stream has `Length` option then we need to cut data for a given length
		int length = (optionList.find("Length") != optionList.end() &&
					  optionList["Length"].find(' ') == std::string::npos)
					 ? stoi(optionList["Length"])
					 : stream.size();
		data = stream.substr(0, length);

		// Find option with instructions for compressing data in current stream and apply
		// corresponding functions for decoding. PDF supports a lot of methods, but text is
		// encoded in three ways: ASCII Hex, ASCII 85-base, GZ/Deflate.
		for (const auto& option : optionList) {
			std::string optionName = option.first;
			if (optionName == "ASCIIHexDecode")
				data = decodeAsciiHex(data);
			else if (optionName == "ASCII85Decode")
				data = decodeAscii85(data);
			else if (optionName == "FlateDecode" || optionName == "FlateDecode]") {
				//std::cout << optionName << " " << optionList["Image"] << std::endl << std::endl;
				data = decodeFlate(data);
			}
			else if (optionName == "CCITTFaxDecode")
				data = decodeCcittFax(data, optionList);
		}
	}

	return data;
}

std::string Pdf::decodeAsciiHex(const std::string& input) const {
	std::string result;

	bool isOdd = true;
	bool isComment = false;

	int codeHigh;
	size_t i;
	for (i = 0, codeHigh = -1; i < input.size() && input[i] != '>'; ++i) {
		char c = input[i];

		if (isComment) {
			if (c == '\r' || c == '\n')
				isComment = false;
			continue;
		}

		switch (c) {
			case '\0': case '\t': case '\r': case '\f': case '\n': case ' ':
				break;
			case '%':
				isComment = true;
				break;
			default: {
				int code = tools::hexCharToDec(c);
				if (code == 0 && c != '0')
					return "";

				if (isOdd)
					codeHigh = code;
				else
					result += (char)(codeHigh * 16 + code);

				isOdd = !isOdd;
				break;
			}
		}
	}

	if (input[i] != '>')
		return "";
	if (isOdd)
		result += (char)(codeHigh * 16);

	return result;
}

std::string Pdf::decodeAscii85(const std::string& input) const {
	std::string result;

	std::unordered_map<int, char> ords;
	bool isComment = false;
	int state = 0;
	int sum   = 0;
	for (size_t i = 0; i < input.size() && input[i] != '~'; ++i) {
		char c = input[i];

		if (isComment) {
			if (c == '\r' || c == '\n')
				isComment = false;
			continue;
		}

		if (c == '\0' || c == '\t' || c == '\r' || c == '\f' || c == '\n' || c == ' ')
			continue;
		if (c == '%') {
			isComment = true;
			continue;
		}
		if (c == 'z' && state == 0) {
			result += "\0\0\0\0";
			continue;
		}
		if (c < '!' || c > 'u')
			return "";

		char code = input[i] & 0xff;
		ords[state++] = code - '!';

		if (state == 5) {
			state = 0;
			for (int sum = 0, j = 0; j < 5; ++j)
				sum = sum * 85 + ords[j];
			for (int j = 3; j >= 0; --j)
				result += (char)(sum >> (j * 8));
		}
	}
	if (state == 1)
		return "";
	else if (state > 1) {
		for (int i = 0, sum = 0; i < state; ++i)
			sum += (ords[i] + (i == state - 1)) * pow(85, 4 - i);
		for (int i = 0; i < state - 1; ++i)
			result += (char)(sum >> ((3 - i) * 8));
	}

	return result;
}

std::string Pdf::decodeFlate(const std::string& input) const {
	unsigned long  originalLength = input.size();
	unsigned long  decodedLength  = originalLength * 2;
	unsigned char* originalString = (unsigned char*)input.c_str();
	unsigned char* decodedString  = (unsigned char*)malloc(decodedLength);

	int status = uncompress(decodedString, &decodedLength, originalString, originalLength);
	while (status == -5) {
		decodedLength *= 2;
		decodedString = (unsigned char*)realloc(decodedString, decodedLength);
		status = uncompress(decodedString, &decodedLength, originalString, originalLength);
	}

	std::string result(decodedString, decodedString + decodedLength);
	free(decodedString);
	return result;
}

std::string Pdf::decodeCcittFax(const std::string& input,
								std::unordered_map<std::string, std::string>& optionList) const
{
	int width = std::stoi(optionList["Width"]);
	int height = std::stoi(optionList["Height"]);
	int CcittGroup = (optionList["K"] == "-1") ? 4 : 3;

	std::string header;
	header += "II";  // Byte order indication: Little indian
	header += writeByte(42, 2);  // Version number (always 42)
	header += writeByte(8,  4);  // Offset to first IFD
	header += writeByte(8,  2);  // Number of tags in IFD
	// ImageWidth, LONG, 1, width
	header += writeByte(256, 2) + writeByte(4, 2) + writeByte(1, 4) + writeByte(width, 4);
	// ImageLength, LONG, 1, length
	header += writeByte(257, 2) + writeByte(4, 2) + writeByte(1, 4) + writeByte(height, 4);
	// BitsPerSample, SHORT, 1, 1
	header += writeByte(258, 2) + writeByte(3, 2) + writeByte(1, 4) + writeByte(1, 4);
	// Compression, SHORT, 1, 4 = CCITT Group 4 fax encoding
	header += writeByte(259, 2) + writeByte(3, 2) + writeByte(1, 4) + writeByte(CcittGroup, 4);
	// Threshholding, SHORT, 1, 0 = WhiteIsZero
	header += writeByte(262, 2) + writeByte(3, 2) + writeByte(1, 4) + writeByte(0, 4);
	// StripOffsets, LONG, 1, length of header
	header += writeByte(273, 2) + writeByte(4, 2) + writeByte(1, 4) + writeByte(108, 4);
	// RowsPerStrip, LONG, 1, lenght
	header += writeByte(278, 2) + writeByte(4, 2) + writeByte(1, 4) + writeByte(height, 4);
	// StripByteCounts, LONG, 1, size of image
	header += writeByte(279, 2) + writeByte(4, 2) + writeByte(1, 4) + writeByte(input.size(), 4);

	return header + input;
}

bool Pdf::getDirtyTextList(std::string stream) {
	bool hasDirtyText = false;
	std::smatch containerList;
	// Extract text containers from pair `BT`-`ET`
	while (std::regex_search(stream, containerList, BT_ET_MASK)) {
		std::string res = containerList[1];
		std::smatch partList;
		// Find text that is displayed by PDF-viewers on the screen. There are many variants,
		// but consider a pair: `[...] TJ` and `Td (...) Tj`
		if (std::regex_search(res, partList, TJ_FONT_MASK))
			m_textList.emplace_back(partList[4], partList[1]);
		else if (std::regex_search(res, partList, TJ_MASK))
			m_textList.emplace_back(partList[1], "1");
		else if (std::regex_search(res, partList, TD_TJ_MASK))
			m_textList.emplace_back(partList[4], partList[1]);

		hasDirtyText = true;
		stream = containerList.suffix().str();
	}

	return hasDirtyText;
}

void Pdf::getTransformationList(const std::string& stream,
								std::unordered_map<std::string, std::string>& transformationList) const
{
	std::smatch sm, sm2;
	// Individual chars
	std::string data = stream;
	while (std::regex_search(data, sm, CHAR_MASK)) {
		// Before the data there is a number indicating the number of lines to read.
		// Take it into account
		size_t count = stoi(sm[1]);
		std::vector<std::string> current = tools::explode(tools::trim(sm[2]), '\n');
		// Read line by line
		for (size_t k = 0; k < count && k < current.size(); ++k) {
			std::string str = tools::trim(current[k]);
			if (std::regex_search(str, sm2, CHAR_TYPE1_MASK))
				transformationList[tools::rpad(sm2[1], 4, '0')] = sm2[2];
		}
		data = sm.suffix().str();
	}

	// Ranges
	data = stream;
	while (std::regex_search(data, sm, RANGE_MASK)) {
		// Before the data there is a number indicating the number of lines to read
		size_t count = stoi(sm[1]);
		std::vector<std::string> current = tools::explode(tools::trim(sm[2]), '\n');
		// Read line by line
		for (size_t k = 0; k < count && k < current.size(); ++k) {
			// Sequence of first type
			std::string str = tools::trim(current[k]);
			if (std::regex_search(str, sm2, RANGE_TYPE1_MASK)) {
				// Convert data to decimal (it's easier to iterate)
				size_t from  = std::stoi(sm2[1], nullptr, 16);
				size_t to    = std::stoi(sm2[2], nullptr, 16);
				size_t _from = std::stoi(sm2[3], nullptr, 16);

				for (size_t m = from, n = 0; m <= to; ++m, ++n)
					transformationList[tools::intToHex(m, 4)] = tools::intToHex(_from + n, 4);
			}
			// Sequence of second type
			else if (std::regex_search(str, sm2, RANGE_TYPE1_MASK)) {
				// Convert data to decimal (it's easier to iterate)
				size_t from  = std::stoi(sm2[1], nullptr, 16);
				size_t to    = std::stoi(sm2[2], nullptr, 16);
				std::vector<std::string> parts = tools::explode(sm2[3], " \n\r\t\b\f", true);

				for (size_t m = from, n = 0; m <= to && n < parts.size(); ++m, ++n)
					transformationList[tools::intToHex(m, 4)] = tools::intToHex(std::stoi(parts[n], nullptr, 16), 4);
			}
		}

		data = sm.suffix().str();
	}
}

void Pdf::getImages(std::string imageData,
					std::unordered_map<std::string, std::string>& optionList,
					pugi::xml_node& htmlNode)
{
	if (!m_extractImages)
		return;

	std::string ext = "png";
	if (optionList.find("DCTDecode") != optionList.end() ||
		optionList.find("DCTDecode]") != optionList.end())
		ext = "jpg";
	else if (optionList.find("JPXDecode") != optionList.end() ||
			 optionList.find("JPXDecode]") != optionList.end())
		ext = "jp2";
	else if (optionList.find("CCITTFaxDecode") != optionList.end() ||
			 optionList.find("CCITTFaxDecode]") != optionList.end())
		ext = "tiff";
	else {
		std::vector<unsigned char> png;
		std::vector<unsigned char> img;
		if (optionList.find("DeviceRGB") != optionList.end()) {
			for (size_t i = 0; i < imageData.size(); ++i) {
				img.emplace_back(imageData[i]);
				if ((i + 1) % 3 == 0)
					img.emplace_back(255);
			}
		}
		else if (optionList.find("DeviceGray") != optionList.end()) {
			for (size_t i = 0; i < imageData.size(); ++i) {
				char c = 255 - imageData[i];
				img.emplace_back(c);
				img.emplace_back(c);
				img.emplace_back(c);
				img.emplace_back(255);
			}
		}
		else if (optionList.find("DeviceCMYK") != optionList.end()) {
			for (size_t i = 0; i < imageData.size(); i += 4) {
				float c = (float)imageData[i] / 100;
				float m = (float)imageData[i + 1] / 100;
				float y = (float)imageData[i + 2] / 100;
				float k = (float)imageData[i + 3];
				c = c * (1 - k) + k;
				m = m * (1 - k) + k;
				y = y * (1 - k) + k;
				img.emplace_back((1 - c) * 255);
				img.emplace_back((1 - m) * 255);
				img.emplace_back((1 - y) * 255);
				img.emplace_back(255);
			}
		}
		imageData.clear();
		lodepng::encode(png, img, stoi(optionList["Width"]), stoi(optionList["Height"]));
		imageData.assign(png.begin(), png.end());
	}

	if (!imageData.empty()) {
		m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));
		// Update attributes
		auto imageNode = htmlNode.append_child("p").append_child("img");
		imageNode.append_attribute("data-tag") = m_imageList.size() - 1;
	}
}

void Pdf::transformText(pugi::xml_node& htmlNode) const {
	for (const auto& textPair : m_textList) {
		const auto& font = m_fontList.at(m_fontNameList.at(textPair.second));
		const auto& transformationList = m_transformationList.at(font.first);

		// We are interested in 2 situations: text in `<>` (hex) and text in `()` (plain-text)
		bool isHex = false;
		bool isPlain = false;

		std::string hex;
		std::string plain;
		std::string document;
		std::string text = textPair.first;
		int textSize = text.size();
		int step = font.second ? 4 : 2;
		for (int j = 0; j < textSize; ++j) {
			char c = text[j];
			switch (c) {
				// Begining of hex-data
				case '<':
					hex = "";
					isHex = true;
					break;
				// End of hex-data
				case '>':
					for (size_t k = 0; k < hex.size(); k += step) {
						std::string chex = tools::rpad(hex.substr(k, step), 4, '0');
						if (transformationList.find(chex) != transformationList.end())
							chex = transformationList.at(chex);
						document += encoding::htmlSpecialDecode(chex); /*html_entity_decode("&#x".chex.";");*/
					}
					isHex = false;
					break;
				// Begining of plain text
				case '(':
					plain = "";
					isPlain = true;
					break;
				// End of plain text
				case ')':
					document += plain;
					isPlain = false;
					break;
				// Escaping symbol
				case '\\': {
					char c2 = text[j + 1];
					if (c2 == '\\' || c2 == '(' || c2 == ')')
						plain += c2;
					else if (c2 == 'n')
						plain += "\n";
					else if (c2 == 'r')
						plain += "\r";
					else if (c2 == 't')
						plain += "\t";
					else if (c2 == 'b')
						plain += "\b";
					else if (c2 == 'f')
						plain += "\f";
					// Octal char code
					else if (c2 >= '0' && c2 <= '9') {
						// We need 3 digits (not more) and only digits
						std::string oct = regex_replace(text.substr(j + 1, 3), TEXT_CHAR_MASK, "");
						j += oct.size() - 1;
						plain += encoding::htmlSpecialDecode(oct, 8); /*html_entity_decode("&#".octdec($oct).";");*/
					}
					++j;
					break;
				}
				default:
					// Add current char to hex-data
					if (isHex)
						hex += c;
					// Add current char to plain text
					if (isPlain) {
						std::string key = tools::intToHex(c, 4);
						if (transformationList.find(key) != transformationList.end())
							plain += encoding::htmlSpecialDecode(transformationList.at(key));
						else
							plain += c;
					}
			}
		}

		// Add text to HTML tags
		auto node = htmlNode.append_child("p");
		node.append_child(pugi::node_pcdata).set_value(document.c_str());
	}
}

template<typename T>
std::string Pdf::writeByte(const T& data, int size, bool isLittleEndian) const {
	std::string result;
	for (int i = 0; i < size; ++i)
		result += (data >> i * 8) & 0xFF;
	if (isLittleEndian)
		std::reverse(result.begin(), result.end());
	return result;
}

}  // End namespace
