/**
 * @brief     Wrapper for cURL library
 * @package   curlwrapper
 * @file      curlwrapper.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @version   1.2
 * @date      23.03.2017 -- 29.10.2017
 */
#pragma once

#include <string>
#include <vector>


/**
 * @namespace curlwrapper
 * @brief
 *     Wrapper for cURL library
 */
namespace curlwrapper {

	/**
	 * @brief
	 *     Get random proxy
	 * @param[in] proxyList
	 *     List of proxy servers
	 * @return
	 *     Proxy link
	 * @since 1.0
	 */
	std::string getRandomProxy(const std::vector<std::string>& proxyList);

	/**
	 * @brief
	 *     cURL callback function which allows save site content to `std::string`
	 * @param[in] contents
	 *     Received content
	 * @param[in] size
	 *     Received content size
	 * @param[in] chunk
	 *     Chunk size
	 * @param[in] str
	 *     Result string
	 * @return
	 *     Size of received data
	 * @since 1.0
	 */
	size_t curlCallbackFunc(void* contents, size_t size, size_t nmemb, std::string* s);

	/**
	 * @brief
	 *     Get site content
	 * @param[out] code
	 *     Status code
	 * @param[in] link
	 *     Site link
	 * @param[in] timeout
	 *     Wait response for N sec
	 * @param[in] attemptCount
	 *     Request attempt count
	 * @param[in] interval
	 *     Interval between requests (sec)
	 * @param[in] userAgent
	 *     User Agent
	 * @param[in] headerList
	 *     List of headers
	 * @param[in] cookieList
	 *     List of cookies
	 * @param[in] proxyList
	 *     List of proxy servers
	 * @return
	 *     Site content
	 * @since 1.0
	 */
	std::string getPageContent(int& code, const std::string& link, long timeout = 20,
							   int attemptCount = 1, int interval = 0,
							   const std::string& userAgent = "",
							   const std::vector<std::string>& headerList = {},
							   const std::vector<std::string>& cookieList = {},
							   const std::vector<std::string>& proxyList = {});

	/**
	 * @brief
	 *     Download and save file
	 * @param[in] link
	 *     File link
	 * @param[in] path
	 *     File new path
	 * @param[in] timeout
	 *     Wait response for N sec
	 * @param[in] attemptCount
	 *     Request attempt count
	 * @param[in] interval
	 *     Interval between requests (sec)
	 * @param[in] userAgent
	 *     User Agent
	 * @param[in] headerList
	 *     List of headers
	 * @param[in] cookieList
	 *     List of cookies
	 * @param[in] proxyList
	 *     List of proxy servers
	 * @since 1.1
	 */
	void downloadFile(const std::string& link, const std::string& path, long timeout = 20,
					  int attemptCount = 1, int interval = 0, const std::string& userAgent = "",
					  const std::vector<std::string>& headerList = {},
					  const std::vector<std::string>& cookieList = {},
					  const std::vector<std::string>& proxyList = {});

	/** OK status code */
	const char STATUS_OK = 0;

}  // End namespace
