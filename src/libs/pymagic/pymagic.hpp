/**
 * @brief     Interface to the libmagic file type identification library
 * @package   pymagic
 * @file      pymagic.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright ahupp (https://github.com/ahupp/python-magic)
 * @version   1.1
 * @date      10.08.2016 -- 29.10.2017
 */
#pragma once

#include <string>


/**
 * @namespace pymagic
 * @brief
 *     Interface to the libmagic file type identification library
 */
namespace pymagic {

	/**
	 * @brief
	 *     Get file extension from path if magiclib fails
	 * @param[in] fileName
	 *     File name
	 * @return
	 *     File extension
	 * @since 1.0
	 */
	std::string getDefaultExtension(const std::string& fileName);

	/**
	 * @brief
	 *     Get file extension using magiclib or `file` command
	 * @param[in] fileName
	 *     File name
	 * @return
	 *     File extension
	 * @since 1.0
	 */
	std::string getFileExtension(const std::string& fileName);

}  // End namespace
