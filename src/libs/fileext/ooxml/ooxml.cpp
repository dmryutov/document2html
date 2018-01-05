/**
 * @brief   Wrapper for Office Open XML (OOXML)
 * @package ooxml
 * @file    ooxml.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    01.01.2017 -- 18.10.2017
 */
#include <iostream>

#include "ooxml.hpp"


namespace ooxml {

void Ooxml::extractFile(const std::string& zipName, const std::string& fileName,
						pugi::xml_document& tree)
{
	mz_zip_archive zipArchive;
	size_t size;
	auto content = getFileContent(zipName, fileName, &zipArchive, size);

	//tree.load_string(static_cast<const char*>(content));
	if (content != nullptr)
		tree.load_buffer(content, size);
	clear(&zipArchive, content);
}

void Ooxml::extractFile(const std::string& zipName, const std::string& fileName,
						std::string& buffer)
{
	mz_zip_archive zipArchive;
	size_t size;
	auto content = getFileContent(zipName, fileName, &zipArchive, size);

	if (content != nullptr)
		buffer = std::string(static_cast<const char*>(content), size);
	clear(&zipArchive, content);
}


// private:
void* Ooxml::getFileContent(const std::string& zipName, const std::string& fileName,
							mz_zip_archive* zipArchive, size_t& size)
{
	// Try to open archive
	size = 0;
	memset(zipArchive, 0, sizeof(*zipArchive));
	auto status = mz_zip_reader_init_file(zipArchive, zipName.c_str(), 0);
	if (!status) {
		//throw std::invalid_argument("Invalid zip file!");
		std::cerr << "std::invalid_argument: Invalid zip file!" << std::endl;
		return nullptr;
	}
	// Unzip file to heap
	auto content = mz_zip_reader_extract_file_to_heap(zipArchive, fileName.c_str(), &size, 0);
	if (!content) {
		//throw std::logic_error("File extracting error!");
		std::cerr << "std::logic_error: File extracting error!" << std::endl;
		return nullptr;
	}
	return content;
}

void Ooxml::clear(mz_zip_archive* zipArchive, void* content) {
	mz_free(content);
	mz_zip_reader_end(zipArchive);
}

}  // End namespace