#include "PG_ZIPUtil.h"

#include <zip.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_ImageLoad.h>

zipUPtr PG_ZIPUtil::openZip(const std::string& file) {
    int error = 0;
    zip* ptr = zip_open(file.c_str(), 0, &error);
    if (ptr == nullptr) {
        OUT_ERROR("Failed to open the zip '" << file << "': error code " << error);
        return zipUPtr(nullptr, [](zip* a) { zip_close(a); });
    }

    return zipUPtr(ptr, [](zip* a) { zip_close(a); });
}


// Utility function to read the contents of a zip file entry into a string
std::string PG_ZIPUtil::readString(zip_file* zf, std::size_t maxSize) {
    std::string content;
    std::vector<char> buffer(1024);
    zip_int64_t bytesRead;
    while ((bytesRead = zip_fread(zf, buffer.data(), buffer.size())) > 0) {
        content.append(buffer.data(), bytesRead);
        if (content.size() > maxSize) {
            throw_RuntimeError("Zip file is bigger then " << maxSize << "!");
        }
    }

    return content;
}

std::vector<char> PG_ZIPUtil::readData(zip_file* zf) {
    std::vector<char> content;
    std::vector<char> buffer(1024);
    zip_int64_t bytesRead;
    constexpr std::size_t maxSize = (1024 * 1024 * 128);//max is 128 MB
    while ((bytesRead = zip_fread(zf, buffer.data(), buffer.size())) > 0) {
        content.insert(content.end(), buffer.begin(), buffer.end());
        if (content.size() > maxSize) {
            throw_RuntimeError("Zip file is bigger the 128 MB!");
        }
    }

    return content;
}

nlohmann::json PG_ZIPUtil::readJson(zip* archive, const std::string& fileInZip) {

    zip_file* zf = zip_fopen(archive, fileInZip.c_str(), 0);
    if (!zf) {
        throw_RuntimeError("Failed to find '" << fileInZip << "' in zip!");
    }

    std::string content = PG_ZIPUtil::readString(zf);
    if (content.empty()) {
        throw_RuntimeError("Failed to read '" << fileInZip << "' in zip!");
    }
    zip_fclose(zf);
    return nlohmann::json::parse(content);
}

PG_RGBAImage PG_ZIPUtil::readPNGImage(zip* archive, const std::string& fileInZip) {

    zip_file* zf = zip_fopen(archive, fileInZip.c_str(), 0);
    if (!zf) {
        throw_RuntimeError("Failed to find '" << fileInZip << "' in zip!");
    }

    auto content = PG_ZIPUtil::readData(zf);
    zip_fclose(zf);

    PG_RGBAImage img;
    if (!PG_ImageLoad::load(content, img) || img.empty()) {
        throw_RuntimeError("Failed to load png img '" << fileInZip << "' from zip!");
    }

    return img;
}

std::vector<char> PG_ZIPUtil::readData(zip* archive, const std::string& fileInZip) {
    zip_file* zf = zip_fopen(archive, fileInZip.c_str(), 0);
    if (!zf) {
        throw_RuntimeError("Failed to find '" << fileInZip << "' in zip! Check that file is written correctly including casing!");
    }

    auto content = PG_ZIPUtil::readData(zf);
    zip_fclose(zf);

    return content;
}

std::string PG_ZIPUtil::readString(zip* archive, const std::string& fileInZip) {
    zip_file* zf = zip_fopen(archive, fileInZip.c_str(), 0);
    if (!zf) {
        throw_RuntimeError("Failed to find '" << fileInZip << "' in zip!");
    }

    auto content = PG_ZIPUtil::readString(zf);
    zip_fclose(zf);

    return content;
}