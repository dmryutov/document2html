/**
 * @brief   Wrapper for archive files
 * @package archive
 * @file    archive.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0
 * @date    09.09.2016 -- 30.10.2017
 */
#pragma once

#include <string>


/**
 * @namespace archive
 * @brief
 *     Wrapper for archive files
 * @details
 *     Console archive tools usage:
 *          Tool    |      File list     | Extract
 *     ------------ | ------------------ | -------
 *     Winrar/Unrar | unrar lb arc_name  | unrar x -y -inul arc_name dir
 *     Unix tar(1)  | tar -tf arc_name   | tar -xf arc_name -C dir
 *     7Z           | 7za.exe l arc_name | 7za.exe x -y arc_name -Odir > nul
 */
namespace archive {

	/**
	 * @brief
	 *     Extract archive to temp directory
	 * @param[in] dirName
	 *     Directory name
	 * @param[in] fileName
	 *     File name
	 * @param[in] extension
	 *     File extension
	 * @param[in] isTempDir
	 *     Output directory type
	 *     Value | Description
	 *     :---: | -----------
	 *     True  | Temp directory
	 *     False | Directory near source file
	 * @return
	 *     Output directory name
	 * @since 1.0
	 */
	std::string extractArchive(const std::string& dirName, const std::string& fileName,
							   const std::string& extension, bool isTempDir = false);

	void extractArchive(const std::string& dirName, const std::string& fileName,
						const std::string& extension, std::string& outputDir);

}  // End namespace
