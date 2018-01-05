/**
 * @brief   Wrapper for archive files
 * @package archive
 * @file    archive.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0
 * @date    09.09.2016 -- 30.10.2017
 */
#include "../../tools.hpp"

#include "archive.hpp"


namespace archive {

using namespace tools;

const std::string LIB_PATH = PROGRAM_PATH + "/files/libs";
#if defined(_WIN32) || defined(_WIN64)
	/** RAR extraction comamnd */
	const std::string RAR_CMD = "\""+ LIB_PATH +"/unrar.exe\" x -y -inul ";
	/** TAR extraction comamnd */
	const std::string TAR_CMD = "\""+ LIB_PATH +"/7za.exe\" x -y ";
#else
	/** RAR extraction comamnd */
	const std::string RAR_CMD = "\""+ LIB_PATH +"/unrar\" x -y -inul ";
	/** TAR extraction comamnd */
	const std::string TAR_CMD = "tar -xf ";
#endif

std::string extractArchive(const std::string& dirName, const std::string& fileName,
						   const std::string& extension, bool isTempDir)
{
	// Create output directory
	std::string mask = PROGRAM_PATH + "/files/temp/XXXXXX";
	tools::createDir(PROGRAM_PATH + "/files/temp");

	std::string outputDir;
	if (isTempDir)
		outputDir = tools::os_mkdtemp(&mask[0]);
	else
		outputDir = dirName +"/"+ fileName +".archive";
	tools::deleteDir(outputDir);
	tools::createDir(outputDir);

	// Extract archive
	std::string path = dirName +"/"+ fileName;
	std::string cmd;
	if (extension == "rar") {
		cmd = RAR_CMD +"\""+ path +"\" \""+ outputDir +"\"";
	}
	else {
		if (IS_WINDOWS) {
			// `gz` and `bz2` archives should be extracted twice
			if (extension == "gz" || extension == "bz2") {
				cmd = TAR_CMD +"\""+ path +"\" -O\""+ outputDir +"\" > nul";
				// Get exclusive access to console
				{
					LOCK lock(MUTEX);
					system(cmd.c_str());
				}
				std::string newName = fileName.substr(0, fileName.size() - extension.size() - 1);
				path = outputDir +"/"+ newName;
				if (isTempDir)
					outputDir = tools::os_mkdtemp(&mask[0]);
				else
					outputDir += "/"+ newName +".archive";
			}
			cmd = TAR_CMD + "\""+ path +"\" -O\""+ outputDir +"\" > nul";
		}
		else {
			cmd = TAR_CMD +"\""+ path +"\" -C \""+ outputDir +"\"";
		}
	}
	// Get exclusive access to console
	{
		LOCK lock(MUTEX);
		system(cmd.c_str());
	}

	return outputDir;
}

void extractArchive(const std::string& dirName, const std::string& fileName,
					const std::string& extension, std::string& outputDir)
{
	// Extract archive
	std::string path = dirName +"/"+ fileName;
	std::string cmd;
	if (extension == "rar") {
		cmd = RAR_CMD +"\""+ path +"\" \""+ outputDir +"\"";
	}
	else {
		if (IS_WINDOWS) {
			// `gz` and `bz2` archives should be extracted twice
			if (extension == "gz" || extension == "bz2") {
				cmd = TAR_CMD +"\""+ path +"\" -O\""+ outputDir +"\" > nul";
				// Get exclusive access to console
				{
					LOCK lock(MUTEX);
					system(cmd.c_str());
				}
				std::string newName = fileName.substr(0, fileName.size() - extension.size() - 1);
				path = outputDir +"/"+ newName;
				outputDir += "/"+ newName +".archive";
			}
			cmd = TAR_CMD + "\""+ path +"\" -O\""+ outputDir +"\" > nul";
		}
		else {
			cmd = TAR_CMD +"\""+ path +"\" -C \""+ outputDir +"\"";
		}
	}
	// Get exclusive access to console
	{
		LOCK lock(MUTEX);
		system(cmd.c_str());
	}
}

}  // End namespace
