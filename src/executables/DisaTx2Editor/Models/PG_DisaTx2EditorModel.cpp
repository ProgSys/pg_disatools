#include "PG_DisaTx2EditorModel.h"

#include <Disa/models/PG_DisaColorTableModel.h>
#include <Util/PG_ImageLoad.h>
#include <Util/PG_StringUtil.h>

#include <algorithm>
#include <unordered_set>

namespace {
	bool isTx2Extension(const PG_File& file) {
		const std::string extension = file.getFileExtensionFormated();
		return extension == "tx2" || extension == "txp";
	}
}

void PG_DisaTx2EditorModel::init() {
	m_colorTableModel = PG_new<PG_DisaColorTableModel>("colorTableModel", this);
	connect(m_colorTableModel, &PG_DisaColorTableModel::currentTableChanged, this, &PG_DisaTx2EditorModel::updateTexture);
	connect(m_colorTableModel, &PG_DisaColorTableModel::colorsModified, this, [this] {
		updateTexture();
		setModified(true);
	});
}

bool PG_DisaTx2EditorModel::open(const PG_File& file, std::string& errorOut) {
	if (file.isEmpty()) return false;
	if (!file.exists()) {
		errorOut = "The file \"" + file.getPath() + "\" doesn't exist!";
		return false;
	}

	PG_TX2 tx2;
	if (isTx2Extension(file)) {
		if (!tx2.open(file.getPath()) || !tx2.isValid()) {
			errorOut = "Failed to open \"" + file.getFile() + "\" as TX2 image!";
			return false;
		}
	}
	else {
		PG_RGBAImage image;
		if (!PG_ImageLoad::load(file.getPath(), image) || image.empty()) {
			errorOut = "Failed to open \"" + file.getFile() + "\" as image!";
			return false;
		}
		if (image.getWidth() > 0xFFFF || image.getHeight() > 0xFFFF) {
			errorOut = "The image is too big for a TX2 image!";
			return false;
		}
		tx2.setWithRGBA(image.getWidth(), image.getHeight(), (const char*)image.data());
	}

	m_tx2 = std::move(tx2);
	m_file = file;
	m_modified = false;
	resetColorTables();
	updateTexture();
	emit imageChanged();
	emit fileChanged();
	emit modifiedChanged();
	return true;
}

bool PG_DisaTx2EditorModel::save(const PG_File& file, std::string& errorOut) {
	if (!isOpen() || file.isEmpty()) return false;

	if (isTx2Extension(file)) {
		if (!m_tx2.save(file.getPath())) {
			errorOut = "Failed to save \"" + file.getPath() + "\"!";
			return false;
		}
		m_file = file;
		setModified(false);
		emit fileChanged();
		return true;
	}

	PG_File pngFile = file;
	if (pngFile.getFileExtensionFormated() != "png") pngFile.replaceFileExtension("png");
	PG_RGBAImage image(m_tx2.getWidth(), m_tx2.getHeight());
	m_tx2.getRGBAData((char*)image.data(), (unsigned short)m_colorTableModel->getCurrentTable());
	if (!PG_ImageLoad::save(pngFile.getPath(), image)) {
		errorOut = "Failed to save \"" + pngFile.getPath() + "\"!";
		return false;
	}
	return true;
}

bool PG_DisaTx2EditorModel::convertTo(PG_TX2::Type type, std::string& errorOut) {
	if (!isOpen()) return false;
	const bool isDxt = type == PG_TX2::DXT1 || type == PG_TX2::DXT5;
	if (isDxt && (m_tx2.getWidth() % 4 != 0 || m_tx2.getHeight() % 4 != 0)) {
		errorOut = "To convert an image to DXT its width and height must be divisible by 4!";
		return false;
	}

	PG_TX2 converted = m_tx2;
	if (!converted.convertTo(type, (unsigned int)m_colorTableModel->getCurrentTable()) || !converted.isValid()) {
		errorOut = "Failed to convert the image!";
		return false;
	}

	m_tx2 = std::move(converted);
	resetColorTables();
	updateTexture();
	setModified(true);
	emit imageChanged();
	return true;
}

bool PG_DisaTx2EditorModel::isTx2File() const {
	return isTx2Extension(m_file);
}

std::string PG_DisaTx2EditorModel::getTypeName() const {
	switch (m_tx2.getType()) {
	case PG_TX2::DXT1: return "DXT1";
	case PG_TX2::DXT4: return "DXT4";
	case PG_TX2::DXT5: return "DXT5";
	case PG_TX2::BGRA: return m_tx2.header.colortableSize == 16 ? "FONT (BGRA 16 colors)" : "BGRA";
	case PG_TX2::FONT: return "FONT";
	case PG_TX2::COLORTABLE_BGRA16: return "COLORTABLE BGRA16";
	case PG_TX2::COLORTABLE_RGBA16: return "COLORTABLE RGBA16";
	case PG_TX2::COLORTABLE_BGRA256: return "COLORTABLE BGRA256";
	case PG_TX2::COLORTABLE_RGBA256: return "COLORTABLE RGBA256";
	default: return "NONE";
	}
}

const std::vector<PG_DisaTx2EditorModel::ConvertTarget>& PG_DisaTx2EditorModel::getConvertTargets() {
	static const std::vector<ConvertTarget> targets = {
		{ "DXT1", PG_TX2::DXT1 },
		{ "DXT5", PG_TX2::DXT5 },
		{ "BGRA8888", PG_TX2::BGRA },
		{ "COLORTABLE RGBA16", PG_TX2::COLORTABLE_RGBA16 },
		{ "COLORTABLE BGRA16", PG_TX2::COLORTABLE_BGRA16 },
		{ "COLORTABLE RGBA256", PG_TX2::COLORTABLE_RGBA256 },
		{ "COLORTABLE BGRA256", PG_TX2::COLORTABLE_BGRA256 },
		{ "FONT", PG_TX2::FONT },
	};
	return targets;
}

std::size_t PG_DisaTx2EditorModel::countColors(std::size_t max) const {
	if (!isOpen()) return 0;
	PG_RGBAImage image(m_tx2.getWidth(), m_tx2.getHeight());
	m_tx2.getRGBAData((char*)image.data(), (unsigned short)m_colorTableModel->getCurrentTable());

	std::unordered_set<uint32_t> colors;
	const uint32_t* pixels = (const uint32_t*)image.data();
	for (std::size_t i = 0, count = std::size_t(image.getWidth()) * image.getHeight(); i < count; ++i) {
		colors.insert(pixels[i]);
		if (colors.size() > max) break;
	}
	return colors.size();
}

void PG_DisaTx2EditorModel::resetColorTables() {
	if (m_tx2.header.colortables.empty()) m_colorTableModel->setTables(nullptr, 0);
	else m_colorTableModel->setTables(&m_tx2.header.colortables, m_tx2.header.colortableSize);
	m_colorTableModel->setTableCountEditable(!m_tx2.header.colortables.empty());
}

void PG_DisaTx2EditorModel::updateTexture() {
	//several changes in a row, like a new table that is also the current one, rebuild the texture once
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaTx2EditorModel::rebuildTexture);
}

void PG_DisaTx2EditorModel::rebuildTexture() {
	if (!isOpen()) {
		m_texture = PG_Texture();
		return;
	}
	PG_RGBAImage image(m_tx2.getWidth(), m_tx2.getHeight());
	m_tx2.getRGBAData((char*)image.data(), (unsigned short)m_colorTableModel->getCurrentTable());
	m_texture.load(PG_UNIQUE_RES("DisaTx2EditorImage"), PG_Texture::ImageFormats{ std::move(image) }, { false, PG_Texture::Mipmap, PG_Texture::Clamp });
	emit imageChanged();
}

void PG_DisaTx2EditorModel::setModified(bool modified) {
	if (m_modified == modified) return;
	m_modified = modified;
	emit modifiedChanged();
}
