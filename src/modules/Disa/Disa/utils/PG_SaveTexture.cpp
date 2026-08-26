#include "PG_SaveTexture.h"
#include <Disa/utils/PG_FileDialog.h>
#include <Util/PG_ImageLoad.h>
#include <Disa/customGui/PG_PupUpDialog.h>

PG_File PG_FileDialog::save(const PG_Texture& texture, const std::string& fileName) {
	if (!texture.isValid()) {
		OUT_ERROR("There is no image to save!");
		return {};
	}
	std::future<PG_Texture::ImageFormats> imageFuture = texture.exportImage();

	PG_File targetFile(fileName);
	targetFile = PG_PopUpDialog::saveAs(targetFile.isEmpty() ? PG_File("preview.png"): std::as_const(targetFile).replaceFileExtension("png"));
	if (!targetFile) return {};
	if (targetFile.getFileExtensionFormated() != "png") targetFile.replaceFileExtension("png");

	std::visit([&targetFile](const auto& img) {
		using T = std::decay_t<decltype(img)>;
		if constexpr (std::is_same_v<T, PG_RGBAImage> || std::is_same_v<T, PG_RGBImage>) {
			if (img.empty()) {
				OUT_ERROR("The preview has no image data to export!");
				return;
			}
			if (!PG_ImageLoad::save(targetFile, img)) {
				OUT_ERROR("Failed to save image as: " << targetFile << "!");
			}
		}
		else {
			OUT_ERROR("The image format of the preview can't be saved as png!");
		}
		}, imageFuture.get());

	return targetFile;
}