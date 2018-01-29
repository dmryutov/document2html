/**
 * @brief     Wrapper for cURL library
 * @package   curlwrapper
 * @file      curlwrapper.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @date      23.03.2017 -- 28.01.2018
 */
#include <fstream>
#include <random>

#include "../tools.hpp"

#if defined(_WIN32) || defined(_WIN64)
	#define CURL_STATICLIB
	#include <curl.h>
#else
	#include <curl/curl.h>
#endif

#include "curlwrapper.hpp"


namespace curlwrapper {

std::random_device rd;
std::default_random_engine gen(rd());

std::string getRandomProxy(const std::vector<std::string>& proxyList) {
	size_t size = proxyList.size();
	if (size == 0)
		return "";
	std::uniform_int_distribution<int> dist(0, (int)size);
	return proxyList[dist(gen)];
}

size_t curlCallbackFunc(void* contents, size_t size, size_t chunk, std::string* str) {
	size_t newLength = size * chunk;
	size_t oldLength = str->size();
	try {
		str->resize(oldLength + newLength);
	}
	catch (std::bad_alloc&) {
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, str->begin() + oldLength);
	return size * chunk;
}

std::string getPageContent(int& code, const std::string& link, long timeout, int attemptCount,
						   int interval, const std::string& userAgent,
						   const std::vector<std::string>& headerList,
						   const std::vector<std::string>& cookieList,
						   const std::vector<std::string>& proxyList)
{
	CURL* curl;
	std::string buffer;
	int attempts = 0;
	code = -1;

	struct curl_slist* hList = NULL;
	for (const auto& header : headerList)
		hList = curl_slist_append(hList, header.c_str());
	std::string cookies = tools::join(cookieList, ";");

	while (code != STATUS_OK && attempts++ < attemptCount) {
		buffer.clear();
		curl = curl_easy_init();
		if (curl) {
			std::string proxy = getRandomProxy(proxyList);

			curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 4L);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallbackFunc);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			if (!userAgent.empty())
				curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
			if (!headerList.empty())
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hList);
			if (!cookies.empty())
				curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
			if (!proxy.empty())
				curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

			// Wait before request
			if (interval > 0)
				tools::sleep(interval);

			// Perform request, `code` will get the return code
			code = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
	}
	return buffer;
}

void downloadFile(const std::string& link, const std::string& path, long timeout,
				  int attemptCount, int interval, const std::string& userAgent,
				  const std::vector<std::string>& headerList,
				  const std::vector<std::string>& cookieList,
				  const std::vector<std::string>& proxyList)
{
	int code;
	std::string fileData = getPageContent(code, link, timeout, attemptCount, interval, userAgent,
										  headerList, cookieList, proxyList);
	if (!fileData.empty()) {
		std::ofstream file(path, std::ios_base::binary);
		file << fileData;
		file.close();
	}
}

}  // End namespace
