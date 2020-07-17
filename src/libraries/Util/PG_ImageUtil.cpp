#include <Util/PG_ImageUtil.h>

#include <algorithm>

using namespace PG::UTIL;

PG::UTIL::RGBAImage PG::UTIL::dumpToImage(const std::vector<char>& dump, int width, int max_height, int offset) {
	const int height = std::min(max_height, (int)std::ceil(std::max( 0 , (int(dump.size())*2 - offset)) / float(width)));
	PG::UTIL::RGBAImage outImage;
	if (height <= 0) return outImage;
	outImage.resize(width * 2, height);

	auto imageIt = outImage.begin();
	auto dumpIt = dump.begin() + offset;

	for (; imageIt != outImage.end() && dumpIt != dump.end(); ++imageIt, ++dumpIt) {
		unsigned char a = ((*dumpIt & 0x0F)/16.0f) * 255.0f;
		unsigned char b = ((*dumpIt >> 4) / 16.0f) * 255.0f;
		*imageIt = PG::UTIL::rgba(a, a, a, 255);
		imageIt++;
		*imageIt = PG::UTIL::rgba(b, b, b, 255);
	}

	return outImage;
}

PG::UTIL::RGBAImage PG::UTIL::dumpToImageRGBA(const std::vector<char>& dump, int width, int max_height, int offset) {
	const int height = std::min(max_height, (int)std::ceil(std::max(0, (int(dump.size()) /4 - offset)) / float(width)));
	PG::UTIL::RGBAImage outImage;
	if (height <= 0) return outImage;
	outImage.resize(width, height);

	auto imageIt = outImage.begin();
	auto dumpIt = dump.begin() + offset;

	for (; imageIt != outImage.end() && dumpIt != dump.end(); ++imageIt) {

		*imageIt = PG::UTIL::rgba(*dumpIt++, *dumpIt++, *dumpIt++, *dumpIt++);
	}

	return outImage;
}

