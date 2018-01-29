/**
 * @brief     PPT files into HTML сonverter
 * @package   ppt
 * @file      ppt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @date      05.08.2017 -- 29.01.2018
 */
#include <list>
#include <regex>
#include <unordered_map>

#include "../../tools.hpp"

#include "ppt.hpp"


namespace ppt {

const std::regex HEADER_MASK("(\xA8|\xA0)\x0F", std::regex::icase);
/** Specific style file path */
//const std::string STYLE_FILE = tools::PROGRAM_PATH + "/files/style/pptStyle.min.css";
const std::string STYLE_FILE = "style.css";

// public:
Ppt::Ppt(const std::string& fileName)
	: FileExtension(fileName), Cfb(fileName) {}

void Ppt::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	auto htmlTag = m_htmlTree.append_child("html");
	auto headTag = htmlTag.append_child("head");
	auto bodyTag = htmlTag.append_child("body");
	FileExtension::loadStyle(headTag, STYLE_FILE);

	Cfb::parse();
	// File must contain `Current User` stream
	std::string cuStream = getStream("Current User");
	if (cuStream.empty())
		return;

	// Check if file is PowerPoint presentation
	if ((unsigned)readByte<int>(cuStream, 12, 4) == 0xF3D1C4DF)
		return;
	// Get offset to first `UserEditAtom` structure
	int offsetToCurrentEdit = readByte<int>(cuStream, 16, 4);

	std::string ppdStream = getStream("PowerPoint Document");
	if (ppdStream.empty())
		return;
	Cfb::clear();

	// Start looking for all `UserEditAtom` structures, which are required to get offsets
	// to `PersistDirectory`
	int offsetLastEdit = offsetToCurrentEdit;
	std::unordered_map<int, int> persistDirEntry;
	std::string live;
	std::list<int> offsetPersistDirectory;
	do {
		std::string userEditAtom = getRecord(ppdStream, offsetLastEdit, 0x0FF5);
		live = userEditAtom;
		offsetPersistDirectory.push_front(readByte<int>(userEditAtom, 12, 4));
		offsetLastEdit = readByte<int>(userEditAtom, 8, 4);
	} while (offsetLastEdit != 0x00000000);

	// Iterate through all offsets
	for (const auto& offset : offsetPersistDirectory) {
		std::string rgPersistDirEntry = getRecord(ppdStream, offset, 0x1772);
		if (rgPersistDirEntry.empty())
			return;
		// Read 4 bytes:
		// - 20 bits is the initial ID of entry in PersistDirectory,
		// - 12 bits are the number of subsequent offsets
		int size = static_cast<int>(rgPersistDirEntry.size());
		for (int k = 0; k < size; ) {
			int persist = readByte<int>(rgPersistDirEntry, k, 4);
			int persistId = persist & 0x000FFFFF;
			int cPersist = ((persist & 0xFFF00000) >> 20) & 0x00000FFF;
			k += 4;

			for (int i = 0; i < cPersist; i++)
				persistDirEntry[persistId + i] = readByte<int>(rgPersistDirEntry, k + i*4, 4);
			k += cPersist * 4;
		}
	}

	// In the last entry, find ID of the entry with `DocumentContainer`
	int docPersistIdRef = readByte<int>(live, 16, 4);
	std::string documentContainer = getRecord(ppdStream, persistDirEntry[docPersistIdRef], 0x03E8);
	// Skip a lot of trash data before `SlideList` structure
	size_t offset = 48;
	std::string trashRecord;
	trashRecord = getRecord(documentContainer, offset, 0x0409);  // exObjList
	if (!trashRecord.empty())
		offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x03F2);  // documentTextInfo
	offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x07E4);  // soundCollection
	if (!trashRecord.empty())
		offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x040B);  // drawingGroup
	offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x0FF0);  // masterList
	offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x07D0);  // docInfoList
	if (!trashRecord.empty())
		offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x0FD9);  // slideHF
	if (!trashRecord.empty())
		offset += trashRecord.size() + 8;
	trashRecord = getRecord(documentContainer, offset, 0x0FD9);  // notesHF
	if (!trashRecord.empty())
		offset += trashRecord.size() + 8;
	trashRecord.clear();

	// Read `SlideList` structure
	int slideCount = 1;
	std::string slideList = getRecord(documentContainer, offset, 0x0FF0);
	std::string text;
	for (size_t i = 0; i < slideList.size(); ) {
		// Add HTML tags
		auto slideDiv = bodyTag.append_child("div");
		slideDiv.append_attribute("class") = "slide";

		std::string slideNumber = "Slide №" + std::to_string(slideCount++);
		auto slideNumberDiv = slideDiv.append_child("div");
		slideNumberDiv.append_attribute("class") = "slide-number";
		slideNumberDiv.append_child(pugi::node_pcdata).set_value(slideNumber.c_str());

		auto slideDataDiv = slideDiv.append_child("div");
		slideDataDiv.append_attribute("class") = "slide-data";


		std::string block = getRecord(slideList, i);
		switch (getRecordType(slideList, static_cast<int>(i))) {
			// RT_SlidePersistAtom (Pointer to slide. Refer to `PersistDirectory` to get this slide)
			case 0x03F3: {
				int pid = readByte<int>(block, 0 , 4);
				std::string slide = getRecord(ppdStream, persistDirEntry[pid], 0x03EE);

				// Skip a lot of trash data before `Drawing` structure
				offset = 32;
				trashRecord = getRecord(slide, offset, 0x03F9);  // slideShowSlideInfoAtom
				if (!trashRecord.empty())
					offset += trashRecord.size() + 8;
				trashRecord = getRecord(slide, offset, 0x0FD9);  // perSlideHFContainer
				if (!trashRecord.empty())
					offset += trashRecord.size() + 8;
				trashRecord = getRecord(slide, offset, 0x3714);  // rtSlideSyncInfo12
				if (!trashRecord.empty())
					offset += trashRecord.size() + 8;

				// `Drawing` is an MS Drawing object that has similar PPT header structure. Search
				// text directly (reading of all possible nested structures is too complicated)
				std::string drawing = getRecord(slide, offset, 0x040C);
				offset += drawing.size() + 8;

				// Try to extract slide title (Office 2003 and earlier)
				offset += 40;  // slideSchemeColorSchemeAtom
				if (getRecordType(slide, static_cast<int>(offset)) == 0x0FBA) {  // slideNameAtom
					std::string title  = unicodeToUtf8(getRecord(slide, offset, 0x0FBA));
					auto slideTitleDiv = slideDiv.insert_child_after("div", slideNumberDiv);
					slideTitleDiv.append_attribute("class") = "slide-title";
					slideTitleDiv.append_child(pugi::node_pcdata).set_value(title.c_str());
				}

				std::smatch offsetList;
				while (std::regex_search(drawing, offsetList, HEADER_MASK)) {
					offset = offsetList.position(1);
					// Check that block header starts with two "zeros", otherwise we probably
					// found something in the middle of other data
					std::string header = drawing.substr(offset - 2, 2);
					if (header[0] == '\x00' && header[1] == '\x00') {
						// Read either Plain text or Unicode
						if (offsetList[1] == (char)0xA8 || offsetList[1] == -88)
							addParagraph(getRecord(drawing, offset - 2, 0x0FA8), slideDataDiv);
						else
							addParagraph(unicodeToUtf8(getRecord(drawing, offset - 2, 0x0FA0)),
										 slideDataDiv);
					}

					drawing = drawing.substr(offset + 2);
				}

				break;
			}
			// RT_TextCharsAtom (Unicode-character occurrence)
			case 0x0FA0: {
				addParagraph(unicodeToUtf8(block), slideDataDiv);
				break;
			}
			// RT_TextBytesAtom (Plain text)
			case 0x0FA8: {
				addParagraph(block, slideDataDiv);
				break;
			}
		}
		// Shift by length of block with title
		i += block.size() + 8;
	}
}


// private:
unsigned short Ppt::getRecordLength(const std::string& stream, size_t offset,
									  unsigned short recType) const
{
	std::string rh = stream.substr(offset, 8);
	if (recType != 0 && recType != readByte<unsigned short>(rh, 2, 2))
		return 0;
	return readByte<int>(rh, 4, 4);
}

unsigned short Ppt::getRecordType(const std::string& stream, int offset) const {
	std::string rh = stream.substr(offset, 8);
	return readByte<unsigned short>(rh, 2, 2);
}

std::string Ppt::getRecord(const std::string& stream, size_t offset, unsigned short recType) const {
	unsigned short length = getRecordLength(stream, offset, recType);
	if (length == 0)
		return "";
	return stream.substr(offset + 8, length);
}

void Ppt::addParagraph(const std::string& text, pugi::xml_node& htmlNode) const {
	/*text = html_entity_decode(iconv("windows-1251", "utf-8", text), ENT_QUOTES, "UTF-8");*/
	auto node = htmlNode.append_child("p");
	node.append_child(pugi::node_pcdata).set_value(text.c_str());
}

}  // End namespace
