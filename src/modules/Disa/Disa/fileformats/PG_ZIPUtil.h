#pragma once

#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <Util/PG_Image.h>
#include <Util/PG_Exception.h>

class zip;
struct zip_file;
using zipUPtr = std::unique_ptr<zip, void(*)(zip*)>;

namespace PG_ZIPUtil{
	/**
	* @brief Will open zip file at given path
	* If open fails then nullptr will be returned
	*/
	zipUPtr openZip(const std::string& file);
	/**
	* @brief Read a string entry form zip. Like a text file.
	* @param maxSize The maximum size of the file, default 128 Mb
	*/
	std::string readString(zip_file* zf, std::size_t maxSize = 1024 * 1024 * 128) throw(PG_RuntimeError);
	/**
	* @brief Read a entry form zip as binary data. Like a image file.
	*/
	std::vector<char> readData(zip_file* zf)  throw(PG_RuntimeError);
	/**
	* @brief Read a given json file from zip.
	*/
	nlohmann::json readJson(zip* archive, const std::string& fileInZip) throw(PG_RuntimeError);
	inline nlohmann::json readJson(const zipUPtr& archive, const std::string& fileInZip) throw(PG_RuntimeError) {
		return readJson(archive.get(), fileInZip);
	}
	/**
	* @brief Returns true when file is present
	*/
	bool hasFile(zip* archive, const std::string& fileInZip);
	inline bool hasFile(const zipUPtr& archive, const std::string& fileInZip) {
		return hasFile(archive.get(), fileInZip);
	}
	/**
	* @brief Read a given png image from zip.
	*/
    PG_RGBAImage readPNGImage(zip* archive, const std::string& fileInZip)  throw(PG_RuntimeError);
	inline PG_RGBAImage readPNGImage(const zipUPtr& archive, const std::string& fileInZip) throw(PG_RuntimeError) {
		return readPNGImage(archive.get(), fileInZip);
	}
	/**
	* @brief Read generic binary data from zip.
	*/
    std::vector<char> readData(zip* archive, const std::string& fileInZip)  throw(PG_RuntimeError);
	inline std::vector<char> readData(const zipUPtr& archive, const std::string& fileInZip) throw(PG_RuntimeError) {
		return readData(archive.get(), fileInZip);
	}
	/**
	* @brief Read generic string data from zip.
	*/
    std::string readString(zip* archive, const std::string& fileInZip)  throw(PG_RuntimeError);
	inline std::string readString(const zipUPtr& archive, const std::string& fileInZip) throw(PG_RuntimeError) {
		return readString(archive.get(), fileInZip);
	}
	/**
	* @brief Will call string for each new line in text
	*/
    void readStringLines(zip* archive, const std::string& fileInZip, const std::function<void(const std::string&)>& callback)  throw(PG_RuntimeError);
}