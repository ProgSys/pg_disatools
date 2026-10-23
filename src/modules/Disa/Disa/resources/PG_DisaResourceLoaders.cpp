#include <Disa/resources/PG_DisaResourceLoaders.h>

#include <Disa/fileformats/PG_TX2.h>
#include <Resources/PG_ResourceManager.h>
#include <Resources/PG_ImageLoader.h>
#include <Util/Debug/PG_DebugOut.h>

void registerDisaResourceLoaders() {
	const PG_ImageLoader tx2Loader{ [](PG_ResourceHandle& handle, std::string_view path) -> std::optional<PG_ImageFormats> {
		PG_RGBAImage image;
		handle.get()->rewind();
		if (decompressTX2(handle.get(), image)) { //returns true on failure
			OUT_ERROR("Failed to decompress tx2: \"" << path << "\"!");
			return std::nullopt;
		}
		return image;
	} };
	PG_ResourceManager::requestRegisterImageLoader("tx2", tx2Loader);
	PG_ResourceManager::requestRegisterImageLoader("txp", tx2Loader);
}
