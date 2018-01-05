/**
 * @brief   Wrapper for Office Open XML (OOXML)
 * @package ooxml
 * @file    ooxml.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.1
 * @date    01.01.2017 -- 18.10.2017
 */
#pragma once

#include <string>

#include "../../miniz/miniz.h"
#include "../../pugixml/pugixml.hpp"


/**
 * @namespace ooxml
 * @brief
 *     Wrapper for Office Open XML (OOXML)
 */
namespace ooxml {

/**
 * @class Ooxml
 * @brief
 *     Wrapper for Office Open XML (OOXML)
 */
class Ooxml {
public:
	Ooxml() = default;

	/**
	 * @brief
	 *     Extract file from archive and put its content into XML-tree
	 * @param[in] zipName
	 *     Archive path
	 * @param[in] fileName
	 *     Extracting file name
	 * @param[out] tree
	 *     XML-tree, where you need to put the data from extracted file
	 * @throw std::invalid_argument
	 *     Invalid zip file
	 * @throw std::logic_error
	 *     File extracting error
	 * @since 1.0
	 */
	static void extractFile(const std::string& zipName, const std::string& fileName,
							pugi::xml_document& tree);

	/**
	 * @brief
	 *     Extract file from archive and put its content into string buffer
	 * @param[in] zipName
	 *     Archive path
	 * @param[in] fileName
	 *     Extracting file name
	 * @param[out] buffer
	 *     String buffer, where you need to put the data from extracted file
	 * @throw std::invalid_argument
	 *     Invalid zip file
	 * @throw std::logic_error
	 *     File extracting error
	 * @since 1.1
	 */
	static void extractFile(const std::string& zipName, const std::string& fileName,
							std::string& buffer);

private:
	/**
	 * @brief
	 *     Get zipped file content
	 * @param[in] zipName
	 *     Archive path
	 * @param[in] fileName
	 *     Extracting file name
	 * @param[in] zipArchive
	 *     Archive handler
	 * @param[out] size
	 *     Extracted file size
	 * @return
	 *     File content
	 * @throw std::invalid_argument
	 *     Invalid zip file
	 * @throw std::logic_error
	 *     File extracting error
	 * @since 1.1
	 */
	static void* getFileContent(const std::string& zipName, const std::string& fileName,
								mz_zip_archive* zipArchive, size_t& size);

	/**
	 * @brief
	 *     Close archive, freeing any resources it was using
	 * @param[in] zipArchive
	 *     Archive handler
	 * @param[in] content
	 *     Extracted file content
	 * @since 1.1
	 */
	static void clear(mz_zip_archive* zipArchive, void* content);
};

}  // End namespace