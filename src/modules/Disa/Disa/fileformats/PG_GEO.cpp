/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include "PG_GEO.h"
#include <algorithm>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInByteArray.h>

#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Image.h>
#include <Util/PG_Exception.h>
#include <Util/PG_S3Compression.h>
#include <Util/PG_ImageLoad.h>
#include <Util/PG_Math.h>
#include <Util/PG_RangesWindow.h>
#include <Util/PG_StringUtil.h>
#include <Disa/PG_DisaTransformation.h>
#include <Script/CScript/PG_CScriptTokenizer.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <tiny_gltf.h>

void geofileTexture::setTypeArgs(PG_TX2::Type type) {
	switch (type)
	{
	default:
	case PG_TX2::TX2ERROR:
		header.type = 0;
		header.colortableSize = 0;
		break;
	case PG_TX2::COLORTABLE_BGRA256:
		header.type = 0;
		break;
	case PG_TX2::COLORTABLE_BGRA16:
		header.type = 0;
		break;
	case PG_TX2::DXT1:
		header.type = 16;
		header.colortableSize = 0;
		break;
	case PG_TX2::DXT5:
		header.type = 16;
		header.colortableSize = 1;
		break;
	case PG_TX2::BGRA:
		header.type = 16;
		header.colortableSize = 3; 
		break;
	}
}

unsigned int geofileTexture::getTotalByteSize() const {
	return sizeof(geofileTextureHeader) + colortable.size() * sizeof(glm::rgba) + pixels.size();
}


PG_RGBAImage geofileTexture::toImageRGBA() const {
	if (header.width == 0 || header.height == 0 || pixels.size() == 0)
		return PG_RGBAImage();


	PG_RGBAImage image(header.width, header.height);
	auto imageIt = image.begin();
	auto setColor = [this, &imageIt](uchar offset) {
		if (offset < colortable.size()) {
			*imageIt = colortable[offset];
			std::swap(imageIt->r, imageIt->b);
		}
		++imageIt;
	};

	switch (type)
	{
	default:
	case PG_TX2::TX2ERROR:
		break;
	case PG_TX2::COLORTABLE_BGRA256: {
		if (header.width * header.height < pixels.size())
			return PG_RGBAImage();
		for (uchar pixel : pixels) {
			setColor(pixel);
			if (imageIt == image.end())
				break;
		}
	}break;
	case PG_TX2::COLORTABLE_BGRA16: {
		if (header.width * header.height < pixels.size())
			return PG_RGBAImage();
		for (uchar halfPixel : pixels) {
			setColor(halfPixel & 0x0f);
			setColor(halfPixel >> 4);
			if (imageIt == image.end())
				break;
		}
	}break;
	case PG_TX2::DXT1: {
		//DXT 1
		const unsigned int numberOfBlocks = pixels.size() / 8;
		std::vector<S3::DXT1block> blocks(numberOfBlocks);
		std::memcpy(blocks.data(), pixels.data(), pixels.size());

		S3::decompressS3(header.width, header.height, blocks, image);
	} break;
	case PG_TX2::DXT5: {
		const unsigned int numberOfBlocks = pixels.size() / 8;
		std::vector<S3::DXT5block> blocks(numberOfBlocks);
		std::memcpy(blocks.data(), pixels.data(), pixels.size());

		S3::decompressS3(header.width, header.height, blocks, image);
	}break;
	case PG_TX2::BGRA: {
		if (pixels.size() != image.byteSize()) return PG_RGBAImage();
		image.fill(glm::rgba(0));
		auto pixelIt = pixels.begin();
		for (; imageIt != image.end(); ++imageIt) {
			imageIt->b = *pixelIt;
			++pixelIt;
			imageIt->g = *pixelIt;
			++pixelIt;
			imageIt->r = *pixelIt;
			++pixelIt;
			imageIt->a = *pixelIt;
			++pixelIt;
		}
	}break;
	}

	return image;
}


// functions
bool PG_GEO::open(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return open(PG_File(file));
}

bool PG_GEO::open(const PG_File& file) {
	if (!file.exists()) {
		clear();
		OUT_ERROR("Target file doesn't exist!");
		return FAILURE;
	}

	try {
		PG::STREAM::InByteFile reader(file);
		const bool result = open(&reader);
		reader.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to import GEO! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import GEO!");
		clear();
		return FAILURE;
	}

}

#define ERROR_TEST(x, message) \
if (x) {\
	clear();\
	OUT_ERROR(message);\
	return FAILURE;\
}

#define WARNING_TEST(x, message) \
if (x) {\
	clear();\
	OUT_WARNING(message);\
	return SUCCESS;\
}

#define WARNING_TEST_LOOP(x, message) \
if (x) {\
	clear();\
	OUT_WARNING(message);\
	break;\
}


bool PG_GEO::open(PG::STREAM::In* reader) {

	clear();

	if (!reader || !reader->isOpen()) {
		OUT_ERROR("Coudn't open file!");
		return FAILURE;
	}

	const unsigned int fileStart = reader->pos();
	const unsigned int fileSize = reader->size() - fileStart; //remaining size

	ERROR_TEST(fileSize < sizeof(geofileHeader) + 32, "Given disa geometry file is too small!");

	reader->read(m_header);

	//Transformations
	ERROR_TEST(m_header.number_of_transformations < 1, "There must be at least one transformation for the root!");
	ERROR_TEST(m_header.number_of_transformations > 1024, "Too many nodes! Total of "<< m_header.number_of_transformations<<".");

	reader->read(m_transformations, m_header.number_of_transformations);

	//Keyframe
	ERROR_TEST(m_header.number_of_keyframes > 1024, "Too many keyframes!");

	reader->read(m_keyframes, m_header.number_of_keyframes);

	//Construction data
	ERROR_TEST(m_header.number_of_nodes < 1, "There must be at least one node for the root!");
	ERROR_TEST(m_header.number_of_nodes > 512, "Too many nodes!");
	reader->read(m_treeNodes, m_header.number_of_nodes);

	//meshes
	ERROR_TEST(m_header.number_of_meshes < 1, "There must be at least one mesh!");
	ERROR_TEST(m_header.number_of_meshes > 100, "Too many meshes!");

	unsigned int mesh_address = fileStart + m_header.offset_to_first_mesh;
	for (unsigned int i = 0; i < m_header.number_of_meshes; ++i) {

		ERROR_TEST(mesh_address - fileStart >= fileSize, "Start of model " << i << " address is outside file!");

		reader->seek(mesh_address);

		geofileMesh model;
		reader->read(model.header);

		ERROR_TEST(model.header.number_of_surfaces < 1, "Model " << i << " must have at least one texture group!");
		ERROR_TEST(model.header.number_of_surfaces > 512, "Model " << i << " has too many texture groups!");

		reader->read(model.surfaces, model.header.number_of_surfaces);

		const unsigned numberOfVertices = model.header.number_of_triangles * 3;
		const unsigned totalTriangleSize = sizeof(glm::vec3) * numberOfVertices;

		ERROR_TEST(model.header.size_of_triangles > totalTriangleSize, "Model " << i << " triangle data is too big!");

		unsigned numberOfUVs = 0;
		int hasNoNormal = 0;
		for (const geofileSurface& tGroup : model.surfaces) {
			if (tGroup.index > 0 && tGroup.length > 0) numberOfUVs += tGroup.length;
			if (tGroup.unk4) ++hasNoNormal;
		}

		const unsigned totalModelSize = totalTriangleSize + sizeof(glm::vec2) * numberOfUVs;
		const int remainingFileSize = fileSize - (reader->pos() - fileStart);

		ERROR_TEST(remainingFileSize < totalModelSize, "Model " << i << " calculated data size is bigger then remainig file size!");

		//read vertices
		model.vertices.resize(numberOfVertices);
		reader->readBytes(model.vertices, model.header.size_of_triangles);

		if (model.header.isNew) {
			model.tangent.resize(numberOfVertices);
			reader->readBytes(model.tangent, model.header.size_of_triangles);
			model.bitangent.resize(numberOfVertices);
			reader->readBytes(model.bitangent, model.header.size_of_triangles);
		}

		//read uv
		reader->read(model.uv, numberOfUVs);



		//read textures
		WARNING_TEST(model.header.start_offset_texture > model.header.end_offset_mesh
			|| mesh_address - fileStart + model.header.start_offset_texture > fileSize
			|| mesh_address - fileStart + model.header.end_offset_mesh > fileSize,
			"Model " << i << " texture offset is bigger then remainig file size!");

		reader->seek(mesh_address + model.header.start_offset_texture);

		int numberOfTextures = model.header.number_of_textures;
		bool nextIsNormal = false;
		for (int t = 0; t < numberOfTextures; ++t) {
			geofileTexture texture;
			reader->read(texture.header);
			WARNING_TEST_LOOP(texture.header.width > 22000 || texture.header.height > 22000, "Model " << i << " texture " << t << " size is too big! At "<<reader->pos()<<".");
			WARNING_TEST_LOOP(texture.header.colortableSize > 512, "Model " << i << " texture " << t << " colortable size is too big!");

			int pixelDataSize = texture.header.width * texture.header.height; // base size
			int colortableSize = texture.header.colortableSize;

			if (nextIsNormal) {
				nextIsNormal = false;
			}else if (model.header.isNew && !nextIsNormal && texture.header.type > 0){
				++numberOfTextures;//has normal?
				nextIsNormal = true;
			}

			texture.type = PG_TX2::TX2ERROR;

			switch (texture.header.type){
			case 0: //colortable
			{
				//colortable
				if (texture.header.colortableSize <= 16) {
					pixelDataSize /= 2; // with 16 colors you only need 4 bit (half byte)
					texture.type = PG_TX2::COLORTABLE_BGRA16;
				}
				else {
					texture.type = PG_TX2::COLORTABLE_BGRA256;
				}
			}break;
			case 1: //Debug
			case 4: 
			case 8: 
			case 32: 
			case 64: 
			{
				texture.type = PG_TX2::BGRA;
				colortableSize = 0;
				pixelDataSize *= 4;
			}break;
			case 16:
				if (texture.header.colortableSize == 0) {
					texture.type = PG_TX2::DXT1;
					colortableSize = 0;
					pixelDataSize /= 2;
				}else if (texture.header.colortableSize == 1) {
					texture.type = PG_TX2::DXT5;
					colortableSize = 0;
				}
				else if (texture.header.colortableSize == 3) {
					texture.type = PG_TX2::BGRA;
					colortableSize = 0;
					pixelDataSize *= 4;
				}else {
					OUT_WARNING("Unknown texture type " << texture.type<< ", "<< texture.header.colortableSize << "!");
				}

				break;

			default:
				OUT_WARNING("Unknown texture type " << texture.type << ", " << texture.header.colortableSize << "!");
				int i = 0;
				break;
			}

			WARNING_TEST_LOOP(texture.type == PG_TX2::TX2ERROR, "Model " << i << " texture " << t << " has unknown type!");
			const int expectedTextureSize = colortableSize * sizeof(glm::rgba) + pixelDataSize;
			//WARNING_TEST_LOOP(texture.header.totalSize < sizeof(geofileTextureHeader) + expectedTextureSize, "Model " << i << " texture " << t << " total pixel data size is too small! At " << reader->pos()<<".");
			WARNING_TEST_LOOP(reader->pos() - fileStart + expectedTextureSize > fileSize, "Model " << i << " texture " << t << " calculated data size is bigger then remainig file size!");

			reader->read(texture.colortable, colortableSize);

			reader->read(texture.pixels, pixelDataSize);
			model.textures.push_back(std::move(texture));
		}

		mesh_address += model.header.end_offset_mesh;
		m_models.push_back(std::move(model));

	}

	return SUCCESS;
}


bool PG_GEO::save(const PG_File& file) const {
	try {
		PG::STREAM::OutByteFile writer(file);
		if(!writer.isOpen()) return FAILURE;
		const bool result = save(&writer);
		writer.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to export GEO! : " << e.what());
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to export GEO!");
		return FAILURE;
	}
}

bool PG_GEO::save(PG::STREAM::Out* writer) const {

	writer->writeData(m_header);
	writer->writeData(m_transformations);
	writer->writeData(m_keyframes);
	writer->writeData(m_treeNodes);

	if (writer->pos() < m_header.offset_to_first_mesh) {
		std::vector<char> fill(m_header.offset_to_first_mesh - writer->pos(), 0);
		writer->writeData(fill);
	}

	unsigned int modelAddress = writer->pos();
	for (const auto& model : m_models) {
		writer->writeData(model.header);
		writer->writeData(model.surfaces);
		writer->writeData(model.vertices);
		if (model.header.isNew) {
			writer->writeData(model.tangent);
			writer->writeData(model.bitangent);
		}
		writer->writeData(model.uv);

		if (writer->pos() < modelAddress + model.header.start_offset_texture) {
			std::vector<char> fill(modelAddress + model.header.start_offset_texture - writer->pos(), 0);
			writer->writeData(fill);
		}

		for (const auto& tex : model.textures) {
			writer->writeData(tex.header);
			writer->writeData(tex.colortable);
			writer->writeData(tex.pixels);
		}

		if (writer->pos() < modelAddress + model.header.end_offset_mesh) {
			std::vector<char> fill(modelAddress + model.header.end_offset_mesh - writer->pos(), 0);
			writer->writeData(fill);
		}
		modelAddress = writer->pos();
	}

	return SUCCESS;
}

static inline void parseSurfaceArgs(geofileSurface& surface, const std::string& name) {
	auto first = PG_StringUtil::findFirst(name, "[");
	if (first == std::string::npos) return;
	auto last = PG_StringUtil::findFirst(name, "]");
	if (last == std::string::npos || first >= last) return;

	std::string args{ name.begin() + first + 1 , name.begin() + last };
	if (args.size() < 2) return;
	PG::STREAM::InByteArray stream(args);
	auto tokens = PG_CScriptTokenizer::tokenize(&stream);

	std::unordered_map<std::string, std::vector<int>> parsedResult;

	using namespace PG_Enum::CScript;
	auto it = tokens.begin();
	while (it != tokens.end()) {
		if (it->type == EmptySpace) {
			++it;
			break;
		}

		if (it->type != Undefined) {
			OUT_ERROR("Parsing parameters failed! Expected a keyword in \""<< name<<"\"!");
			break;
		}

		auto& params = parsedResult[it->str];
		params.clear();
		++it;
		if (it == tokens.end()) break;
		
		if (it->type == Semikolon || it->type == Comma) {
			++it;
			continue;
		}
		//read parameters
		if (it->type != BracketOpen) {
			OUT_ERROR("Parameters must begin via a '(' but got '"<< it->str <<"'! In \"" << name << "\"!");
			break;
		}
		++it;
		if (it == tokens.end()) break;

		while (it != tokens.end()) {
			try {
				params.push_back(PG_StringUtil::toInt(it->str));
			}
			catch (...) {
				OUT_ERROR("Failed to parse number in \"" << name << "\"!");
				params.clear();
				return;
			}

			++it;
			if (it == tokens.end()) return;
			if (it->type == BracketClose) break;

			if (it->type != Comma) {
				OUT_ERROR("Parameters must be seperated via a ',' but got '" << it->str << "'! In \"" << name << "\"!");
				return;
			}
			++it;
		}
		if (it == tokens.end()) return;
		if (it->type != BracketClose) {
			OUT_ERROR("Parameters must end via a ')' but got '" << it->str << "'! In \"" << name << "\"!");
			break;
		}
		++it;
		if (it == tokens.end()) break;

		if (it->type == Semikolon || it->type == Comma) {
			++it;
			continue;
		}
		break;
	}

	for (const auto& [key, params] : parsedResult) {
		if (key == "a" || key == "animated") {
			surface.animated = 1;
			continue;
		}

		if (key == "ad" || key == "additive") {
			surface.additive = params.size() > 0? glm::clamp(params.front(), 0, 3): 1;
			continue;
		}

		if (key == "rgba") {
			if (params.size() != 4) {
				OUT_ERROR("Rgba expects 4 parameters! Got " << params.size() << "! In \"" << name << "\"!");
				continue;
			}
			surface.rMult = glm::clamp(params[0], 0, 255);
			surface.gMult = glm::clamp(params[1], 0, 255);
			surface.bMult = glm::clamp(params[2], 0, 255);
			surface.aMult = glm::clamp(params[3], 0, 255);
			continue;
		}
	}

}

/**
* @brief Helper struct to parse a gltf into a geo obj
*/
struct GLTFToGEOParser {
	PG_File file;
	const tinygltf::Model& modelGLTF;
	std::vector<geofileTreeNode> geoNodes;
	std::vector<geofileTransformation> geoTransformation;
	std::vector<geofileMesh> geoModels;
	PG_CScriptTokenizer tokenizer;

	static inline glm::vec3 toVec3(const std::vector<double>& vector, float defaultValue = 0.f) {
		glm::vec3 out(defaultValue);
		if (vector.size() >= 3) {
			out = { vector[0], vector[1], vector[2] };
		}
		else if (!vector.empty()) {
			if (vector.size() >= 1) out.x = vector[0];
			if (vector.size() >= 2) out.y = vector[1];
		}
		return out;
	}

	static inline glm::vec4 toVec4(const std::vector<double>& vector) {
		glm::vec4 out(0.f);
		if (vector.size() >= 4) {
			out = { vector[0], vector[1], vector[2] , vector[3] };
		}
		else if (!vector.empty()) {
			if (vector.size() >= 1) out.x = vector[0];
			if (vector.size() >= 2) out.y = vector[1];
			if (vector.size() >= 3) out.z = vector[2];
		}
		return out;
	}

	static inline geofileTransformation toGeofileTransformation(const tinygltf::Node& node) {
		PG_Transformation transformPG(toVec3(node.translation), glm::quat(toVec4(node.rotation)), toVec3(node.scale, 1.f));

		geofileTransformation trans;
		trans.position = transformPG.getTranslation() * 10.f;
		glm::vec3 eulerAnglesDegree = transformPG.getEulerAngles();
		trans.rotation = { eulerAnglesDegree.y, eulerAnglesDegree.x, eulerAnglesDegree.z };
		trans.scale = transformPG.getScale() * 100.f;
		trans.times = glm::vec3(0.f);

		trans.unk7 = 0.f;
		trans.unk8 = 0.f;
		trans.unk9 = 0.f;
		trans.unk13 = 0.f;

		return trans;
	}

	geofileMesh parseMesh(const tinygltf::Mesh& mesh) const {
		geofileMesh geoModel;
		geoModel.header.number_of_triangles = 0;
		geoModel.header.isNew = false;
		geoModel.header.unk9 = 0;
		geoModel.header.number_of_surfaces = 0;
		geoModel.header.start_offset_texture = 0;
		geoModel.header.end_offset_mesh = 0;
		geoModel.header.size_of_triangles = 0;
		geoModel.header.unk12 = 0;


		std::vector<unsigned int> indices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normal; //TODO not used

		for (const auto& primitive : mesh.primitives) {
			if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
				continue;
			if (primitive.material < 0 || primitive.material >= modelGLTF.materials.size()) {
				OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as it has NO material!");
				continue;
			}

			const auto& material =  modelGLTF.materials[primitive.material];
			if (auto findIt = material.values.find("baseColorTexture"); findIt != material.values.end()) {
				auto textureIndex = findIt->second.TextureIndex();
				if (textureIndex < 0 || textureIndex >=  modelGLTF.textures.size()) {
					OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as it has NO texture in material \"" << material.name << "\"!");
					continue;
				}
				const auto& texture =  modelGLTF.textures[textureIndex];
				const auto& image =  modelGLTF.images[texture.source];

				if (image.name.empty()) {
					OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as image has NO name in material \"" << material.name << "\"!");
					continue;
				}
				PG_TX2 tx2;


				if (auto replacement = file.replaceFileNameAndExtension(image.name + ".tx2");  replacement.exists()) {
					if (!tx2.open(replacement)) {
						OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as failed to load replacemnt tx2 at \"" << replacement << "\"!");
						continue;
					}
				}
				else {
					if (image.mimeType != "image/png") {
						OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as image type is NOT PNG in material \"" << material.name << "\"!");
						continue;
					}

					const auto& view =  modelGLTF.bufferViews[image.bufferView];
					const auto& buffer =  modelGLTF.buffers[view.buffer];

					std::vector<unsigned char> imageData(view.byteLength);
					memcpy(imageData.data(), buffer.data.data() + view.byteOffset, view.byteLength);

					PG_RGBAImage rgbaImage;
					if (!PG_ImageLoad::load(imageData, rgbaImage) || rgbaImage.getWidth() == 0 || rgbaImage.getHeight() == 0) {
						OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as failed to load PNG image in material \"" << material.name << "\"!");
						continue;
					}

					tx2.setWithRGBA(rgbaImage.getWidth(), rgbaImage.getHeight(), (char*)rgbaImage.data());
					rgbaImage.clear();
				}


				geofileTexture geoTexture;
				switch (tx2.getType())
				{
				case PG_TX2::DXT1: geoTexture.type = PG_TX2::DXT1; break;
				case PG_TX2::DXT5: geoTexture.type = PG_TX2::DXT5; break;
				case PG_TX2::BGRA: geoTexture.type = PG_TX2::BGRA;  break;
				case PG_TX2::COLORTABLE_BGRA16:
				case PG_TX2::COLORTABLE_RGBA16:
					geoTexture.type = PG_TX2::COLORTABLE_BGRA16; break;
				case PG_TX2::COLORTABLE_BGRA256:
				case PG_TX2::COLORTABLE_RGBA256:
					geoTexture.type = PG_TX2::COLORTABLE_BGRA256; break;
				case PG_TX2::DXT4:
				case PG_TX2::FONT: {
					OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as texture format is NOT supported in material \"" << material.name << "\"!");
				}
				default:
					break;
				}


				geoTexture.header.width = tx2.getWidth();
				geoTexture.header.height = tx2.getHeight();
				geoTexture.header.powerWidth = PG_Math::nextPowerOfTwoExponent(tx2.getWidth());
				geoTexture.header.powerHeight = PG_Math::nextPowerOfTwoExponent(tx2.getHeight());
				geoTexture.header.colortableSize = tx2.header.colortableSize;
				geoTexture.header.hasNormal = 0;
				geoTexture.setTypeArgs(geoTexture.type);
				geoTexture.header.unk0 = 0;
				geoTexture.header.unk4 = 0;
				if (!tx2.header.colortables.empty()) geoTexture.colortable = std::move(tx2.header.colortables.front());
				geoTexture.pixels = std::move(tx2.data);
				geoTexture.header.totalSize = geoTexture.pixels.size() + geoTexture.colortable.size() * sizeof(glm::rgba) + sizeof(geofileTextureHeader);

				geoModel.textures.push_back(std::move(geoTexture));
			}
			else {
				OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as it has no \"baseColorTexture\" in material \"" << material.name << "\"!");
				continue;
			}

			indices.clear();
			vertices.clear();
			uv.clear();
			normal.clear();

			//index
			if (primitive.indices > 0) {
				const auto& accessor =  modelGLTF.accessors[primitive.indices];
				const auto& view =  modelGLTF.bufferViews[accessor.bufferView];
				const auto& buffer =  modelGLTF.buffers[view.buffer];

				if (accessor.type == TINYGLTF_TYPE_SCALAR) {
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						std::vector<unsigned short> indicesShort;
						indicesShort.resize(view.byteLength / sizeof(unsigned short));
						memcpy(indicesShort.data(), buffer.data.data() + view.byteOffset, view.byteLength);

						indices = { indicesShort.begin(), indicesShort.end() };
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						indices.resize(view.byteLength / sizeof(unsigned int));
						memcpy(indices.data(), buffer.data.data() + view.byteOffset, view.byteLength);
					}

				}
			}

			//position
			auto it = primitive.attributes.find("POSITION");
			if (it != primitive.attributes.end()) {
				const auto& accessor =  modelGLTF.accessors[it->second];
				const auto& view =  modelGLTF.bufferViews[accessor.bufferView];
				const auto& buffer =  modelGLTF.buffers[view.buffer];

				if (accessor.type == TINYGLTF_TYPE_VEC4) {
					std::vector<glm::vec4> v;
					v.resize(view.byteLength / sizeof(glm::vec4));
					memcpy(v.data(), buffer.data.data() + view.byteOffset, view.byteLength);

					vertices = { v.begin(), v.end() };

				}
				else if (accessor.type == TINYGLTF_TYPE_VEC3) {
					vertices.resize(view.byteLength / sizeof(glm::vec3));
					memcpy(vertices.data(), buffer.data.data() + view.byteOffset, view.byteLength);
				}
			}

			//normal
			it = primitive.attributes.find("NORMAL");
			if (it != primitive.attributes.end()) {
				const auto& accessor =  modelGLTF.accessors[it->second];
				const auto& view =  modelGLTF.bufferViews[accessor.bufferView];
				const auto& buffer =  modelGLTF.buffers[view.buffer];
				if (accessor.type == TINYGLTF_TYPE_VEC3) {
					normal.resize(view.byteLength / sizeof(glm::vec3));
					memcpy(normal.data(), buffer.data.data() + view.byteOffset, view.byteLength);
				}
			}

			//uv
			it = primitive.attributes.find("TEXCOORD_0");
			if (it != primitive.attributes.end()) {
				const auto& accessor =  modelGLTF.accessors[it->second];
				const auto& view =  modelGLTF.bufferViews[accessor.bufferView];
				const auto& buffer =  modelGLTF.buffers[view.buffer];

				if (accessor.type == TINYGLTF_TYPE_VEC2) {
					uv.resize(view.byteLength / sizeof(glm::vec2));
					memcpy(uv.data(), buffer.data.data() + view.byteOffset, view.byteLength);
				}
			}

			//flatten result, as geo format dosn't support indices
			if (!indices.empty()) {
				if (indices.size() % 3 != 0) {
					OUT_WARNING("Mesh \"" << mesh.name << "\" is skipped as indices do not construct triangles! Index count is " << indices.size() << " and not divisible by 3!");
					continue;
				}

				std::vector<glm::vec3> verticesFlat;
				std::vector<glm::vec2> uvFlat;
				std::vector<glm::vec3> normalFlat;
				verticesFlat.reserve(indices.size());
				uvFlat.reserve(indices.size());
				normalFlat.reserve(indices.size());

				for (auto [i0, i1, i2] : indices | PG_RangeSegmentWindow<3>) {
					verticesFlat.push_back(vertices[i1]);
					verticesFlat.push_back(vertices[i0]);
					verticesFlat.push_back(vertices[i2]);
					uvFlat.push_back(uv[i1]);
					uvFlat.push_back(uv[i0]);
					uvFlat.push_back(uv[i2]);
					normalFlat.push_back(normal[i1]);
					normalFlat.push_back(normal[i0]);
					normalFlat.push_back(normal[i2]);
				}
				indices.clear();
				vertices = std::move(verticesFlat);
				uv = std::move(uvFlat);
				normal = std::move(normalFlat);
			}
			if (vertices.empty()) continue;

			//swap space
			for (glm::vec3& v : vertices) {
				v.x *= -1.f;
				v.y *= -1.f;
			}

			//add to model
			geofileSurface surface{ 0,0,0,0,0,0,0,0,0,0,0 };
			surface.index = geoModel.surfaces.size() + 1;
			surface.length = vertices.size();
			surface.rMult = 255;
			surface.gMult = 255;
			surface.bMult = 255;
			surface.aMult = 255;
			parseSurfaceArgs(surface, mesh.name);
			
			geoModel.surfaces.push_back(surface);
			geoModel.vertices.insert(geoModel.vertices.end(), vertices.begin(), vertices.end());
			geoModel.uv.insert(geoModel.uv.end(), uv.begin(), uv.end());



		}

		return geoModel;
	}

	void parseNodes(short parentIndex, const std::vector<int>& gltfNodesIndeces) {
		for (int nodeIndex : gltfNodesIndeces) {
			if (nodeIndex < 0 || nodeIndex >= modelGLTF.nodes.size()) {
				OUT_WARNING("GTLF has an invalid node reference.");
				continue;
			}

			const tinygltf::Node& node = modelGLTF.nodes[nodeIndex];
			//node.mesh
			geoNodes.emplace_back(parentIndex, static_cast<short>(geoTransformation.size()), 1, 0, 0, 0, 0);
			geoTransformation.emplace_back(toGeofileTransformation(node));

			if (node.mesh >= 0) {
				auto geoModel = parseMesh(modelGLTF.meshes[node.mesh]);
				if (!geoModel.surfaces.empty()) {
					geoNodes.back().model_index = geoModels.size();
					geoModels.push_back(std::move(geoModel));
				}
			}

			if (node.children.size() > 0)
				parseNodes(geoNodes.size() + 1, node.children);

		}
	}

	void parse() {
		parseNodes(0, modelGLTF.scenes.front().nodes);
	}
};

bool PG_GEO::importGLTF(const PG_File& file) {
	clear();

	tinygltf::Model modelGLTF;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	bool ret = loader.LoadBinaryFromFile(&modelGLTF, &err, &warn, file.getPath());
	if (!ret || !err.empty()) {
		OUT_ERROR("Failed to parse: \"" + file.getPath() + "\"");
		OUT_ERROR("Message: " << err);
		return FAILURE;
	}
	if (!warn.empty()) OUT_WARNING(warn);

	if (modelGLTF.scenes.empty()) {
		OUT_ERROR("GLTF has no scene!");
		return FAILURE;
	}

	if (modelGLTF.scenes.front().nodes.empty()) {
		OUT_ERROR("GLTF scene has no nodes!");
		return FAILURE;
	}

	GLTFToGEOParser parser{ file, modelGLTF };
	parser.parse();

	getTransformations() = std::move(parser.geoTransformation);
	getTreeNodes() = std::move(parser.geoNodes);
	getModels() = std::move(parser.geoModels);
	computeAddresses();
	return SUCCESS;
}

void PG_GEO::clear() {
	m_header.offset_to_first_mesh = 0;
	m_transformations.clear();
	m_keyframes.clear();
	m_models.clear();
}

void PG_GEO::dump(const std::string& file) const {
	std::ofstream myfile;

	myfile.open(file.c_str());

	myfile << "Header:\n";

	myfile.close();
}

#include <queue>
PG_AABB PG_GEO::computeAABB() const {
	PG_AABB aabb;
	struct TreeItem {
		const geofileTreeNode* node;
		PG_Transformation mat;
		TreeItem* parent;
		std::vector<TreeItem*> children;
	};

	std::vector<TreeItem> tree;

	for (const geofileTreeNode& tNode : getTreeNodes()) {
		const auto& trans = getTransformations()[glm::clamp((int)tNode.transformation_index, 0, (int)getTransformations().size() - 1)];
		PG_DisaTransformation pgTrans(
			fromDisaPosition(trans.position),
			fromDisaRotationEuler(trans.rotation),
			fromDisaScale(trans.scale)
		);

		tree.emplace_back(&tNode, std::move(pgTrans), nullptr);
	}


	for (TreeItem& treeItem : tree) {
		if (treeItem.node->parent_index < 0) {
			TreeItem& parent = tree[treeItem.node->parent_index];
			parent.children.push_back(&treeItem);
			treeItem.parent = &parent;
		}
	}

	std::queue<TreeItem*> queue;
	for (TreeItem& treeItem : tree) {
		if (!treeItem.parent) queue.push(&treeItem);
	}

	while (!queue.empty()){
		auto* node = queue.front();
		queue.pop();
		if (node->parent) {
			node->mat = node->parent->mat * node->mat;
		}

		if (node->node->model_index >= 0) {
			const auto mat = node->mat.getMatrix();
			const int modelIndex = glm::clamp((int)node->node->model_index, 0, (int)getModels().size() - 1);
			const auto& model = getModels()[modelIndex];
			for (const glm::vec3& v : model.vertices) {
				aabb.add(glm::vec3(mat * glm::vec4(-v.x, -v.y, v.z, 1.f)));
			}
		}
	}

	return aabb;
}

void PG_GEO::computeAddresses() {

	m_header.unk1 = 0;
	m_header.number_of_nodes = m_treeNodes.size();
	m_header.unk3 = -1;
	m_header.number_of_transformations = m_transformations.size();
	m_header.number_of_keyframes = m_keyframes.size();
	m_header.number_of_meshes = m_models.size();
	m_header.unk6 = 0;

	unsigned int fillSize =
		sizeof(geofileHeader)
		+ sizeof(geofileTransformation) * m_header.number_of_transformations
		+ sizeof(geofileKeyframe) * m_header.number_of_keyframes
		+ sizeof(geofileTreeNode) * m_header.number_of_nodes;

	if (int mod = fillSize % 16; mod > 0) fillSize += 16 - mod;
	m_header.offset_to_first_mesh = fillSize;

	for (auto& model : m_models) {
		model.header.number_of_surfaces = model.surfaces.size();
		model.header.number_of_triangles = model.vertices.size()/3;
		model.header.size_of_triangles = model.vertices.size() * sizeof(glm::vec3);
		model.header.number_of_textures = model.textures.size();

		//surfaces with index 0 carry no uvs, so there can be fewer uvs than corners. save() writes model.uv, so reserve exactly that
		fillSize = sizeof(geofileMeshHeader)
			+ sizeof(geofileSurface) * model.header.number_of_surfaces
			+ sizeof(glm::vec3) * model.header.number_of_triangles * 3
			+ sizeof(glm::vec2) * model.uv.size();
		if (model.header.isNew) {
			fillSize += sizeof(glm::vec3) * model.header.number_of_triangles * 3 * 2; //tangent and  bitangent
		}
		if (int mod = fillSize % 16; mod > 0) fillSize += 16 - mod;
		model.header.start_offset_texture = fillSize;
		for (auto& texture : model.textures) {
			fillSize += texture.getTotalByteSize();
		}
		if (int mod = fillSize % 16; mod > 0) fillSize += 16 - mod;

		model.header.end_offset_mesh = fillSize;
	}

}