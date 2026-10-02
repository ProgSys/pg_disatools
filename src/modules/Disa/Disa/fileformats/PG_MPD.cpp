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

#include "PG_MPD.h"
#include <algorithm>
#include <iostream>
#include <Disa/fileformats/PG_MPDCustom.h>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>

#include <Util/PG_Image.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>

#include <nlohmann/json.hpp>

//a facing reads as its compass name in json, an unknown one falls back to the first entry
NLOHMANN_JSON_SERIALIZE_ENUM(EDisaFacing, {
	{ DisaFacing_North, "North" },
	{ DisaFacing_East, "East" },
	{ DisaFacing_South, "South" },
	{ DisaFacing_West, "West" },
	});

 // functions
bool PG_MPD::open(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return open(PG_File(file));
}

bool PG_MPD::open(const PG_File& file) {
	clear();
	if (!file.exists()) {
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
		OUT_ERROR("Failed to import MPD! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import MPD!");
		clear();
		return FAILURE;
	}

	return SUCCESS;
}


bool PG_MPD::open(PG::STREAM::In* reader) {
	clear();
	if (!reader || !reader->isOpen()) {
		OUT_ERROR("Reader is not open!");
		return FAILURE;
	}

	const unsigned int file_size = reader->size();
	if (file_size < 32) {
		OUT_ERROR("Given file is too small!");
		return FAILURE;
	}

	const unsigned int start = reader->pos();
	reader->read((char*)&m_header, sizeof(mpdfileHeader));

	if (m_header.number_of_chunks == 0) {
		clear();
		OUT_ERROR("The map is empty!");
		return FAILURE;
	}
	if (m_header.number_of_chunks > 900) {
		clear();
		OUT_ERROR("The map has too many chunks!");
		return FAILURE;
	}

	if (m_header.number_of_actors > 900) {
		clear();
		OUT_ERROR("The map has too many actors!");
		return FAILURE;
	}

	for (unsigned int i = 0; i < m_header.number_of_chunks; i++) {
		mpdfileChunk chunk;
		reader->read((char*)&chunk.info, sizeof(mpdfileChunkDef));

		if (chunk.info.header.number_of_tiles > 900) {
			OUT_ERROR("The chunk " << i << " has too many tiles!");
			continue;
		}
		m_chunks.push_back(std::move(chunk));
	}


	for (mpdfileChunk& chunk : m_chunks) {
		chunk.tiles.resize(chunk.info.header.number_of_tiles);
		reader->read((char*)chunk.tiles.data(), chunk.tiles.size() * sizeof(mpdfileTile));
	}

	m_actors.reserve(m_header.number_of_actors);
	for (unsigned int i = 0; i < m_header.number_of_actors; i++) {
		mpdfileActor actor;
		reader->read((char*)&actor, sizeof(mpdfileActor));
		m_actors.push_back(std::move(actor));
	}

	//read custom data
	m_customMetaData.reset();
	if (const auto blockStart = findMetaDataBlock(reader, start, reader->pos())) {
		reader->seek(*blockStart + 4);
		m_customMetaData.open(reader);
	}

	for (mpdfileChunk& chunk : m_chunks) {
		//callers only ever see regular objects, the marker is the binary file's business
		for (mpdfileChunkObject& object : chunk.info.objects) {
			if (!object.isCustomModelsMarker()) continue;
			object = mpdfileChunkObject{};
		}
	}

	return SUCCESS;
}

std::optional<unsigned int> PG_MPD::findMetaDataBlock(PG::STREAM::In* reader, unsigned int start, unsigned int actorsEnd) {
	const unsigned int end = reader->size();

	//the trailer holds where "PGMD" starts, counted from the start of the MPD
	if (end >= actorsEnd + 8) {
		reader->seek(end - 4);
		const unsigned int blockStart = start + reader->readUnsignedInt();
		if (blockStart >= actorsEnd && blockStart + 8 <= end) {
			reader->seek(blockStart);
			if (reader->readString(4) == "PGMD") return blockStart;
		}
	}

	//files written before the trailer existed have the block right after the actors
	if (actorsEnd + 5 < end) {
		reader->seek(actorsEnd);
		if (reader->readString(4) == "PGMD") return actorsEnd;
	}
	return std::nullopt;
}

bool PG_MPD::openJson(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return openJson(PG_File(file));
}

bool PG_MPD::openJson(const PG_File& file) {
	clear();
	if (!file.exists()) {
		OUT_ERROR("Target file doesn't exist!");
		return FAILURE;
	}

	try {
		PG::STREAM::InByteFile reader(file);
		const bool result = openJson(&reader);
		reader.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to import JSON! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import JSON!");
		clear();
		return FAILURE;
	}

	return SUCCESS;
}

static inline mpdfileTile jsonToTile(const nlohmann::json& jTile) {
	mpdfileTile tile;

	const auto& tTextures = jTile["textures"];
	unsigned int t = 0;
	for (const auto& tTexture : tTextures) {
		tTexture["u"].get_to(tile.textures[t].u);
		tTexture["v"].get_to(tile.textures[t].v);
		tTexture["vRight"].get_to(tile.textures[t].vRight);
		tTexture["width"].get_to(tile.textures[t].width);
		tTexture["height"].get_to(tile.textures[t].height);
		tTexture["heightRight"].get_to(tile.textures[t].heightRight);
		tTexture["texture_index"].get_to(tile.textures[t].textureIndex);
		tTexture["transform"].get_to(tile.textures[t].transform);
		++t;
		if (t >= 12) break;
	}


	{
		const auto& corners = jTile["corners_top"];
		corners["SW"].get_to(tile.cornersTop[0]);
		corners["SE"].get_to(tile.cornersTop[1]);
		corners["NW"].get_to(tile.cornersTop[2]);
		corners["NE"].get_to(tile.cornersTop[3]);
	}
	{
		auto& corners = jTile["corners_sideN"];
		corners["a"].get_to(tile.sideN.x);
		corners["b"].get_to(tile.sideN.y);
	}
	{
		auto& corners = jTile["corners_sideE"];
		corners["a"].get_to(tile.sideE.x);
		corners["b"].get_to(tile.sideE.y);
	}
	{
		auto& corners = jTile["corners_sideS"];
		corners["a"].get_to(tile.sideS.x);
		corners["b"].get_to(tile.sideS.y);
	}
	{
		auto& corners = jTile["corners_sideW"];
		corners["a"].get_to(tile.sideW.x);
		corners["b"].get_to(tile.sideW.y);
	}

	{
		for (int i = 0; i < 4; ++i) {
			jTile["unknown_" + std::to_string(i + 1)].get_to(tile.unk2[i]);
		}
	}
	jTile["visible"].get_to(tile.visible);
	jTile["x"].get_to(tile.x);
	jTile["z"].get_to(tile.z);
	{
		for (int i = 0; i < 4; ++i) {
			jTile["unknown_" + std::to_string(i + 1 + 4)].get_to(tile.unk3[i]);
		}
	}
	jTile["mobility"].get_to(tile.mobility);
	jTile["geo_color"].get_to(tile.geo_color);
	jTile["geo_mark"].get_to(tile.geo_mark);
	{
		for (int i = 0; i < 6; ++i) {
			jTile["padding_" + std::to_string(i + 1)].get_to(tile.padding[i]);
		}
	}
	return tile;
}


bool PG_MPD::openJson(PG::STREAM::In* reader) {
	clear();
	if (!reader || !reader->isOpen()) {
		OUT_ERROR("Reader is not open!");
		return FAILURE;
	}

	std::string str;
	str.resize(reader->size());
	reader->read(str.data(), str.size());
	nlohmann::json j = nlohmann::json::parse(str);
	if (j.empty()) {
		OUT_ERROR("Failed to parse JSON!");
		return FAILURE;
	}

	if (auto header = j.find("header"); header != j.end() && header->is_object()) {
		(*header)["number_of_chunks"].get_to(m_header.number_of_chunks);
		(*header)["number_of_actors"].get_to(m_header.number_of_actors);
		(*header)["field_0x4"].get_to(m_header.field_0x4);
		(*header)["field_0x6"].get_to(m_header.field_0x6);
		(*header)["field_0x8"].get_to(m_header.field_0x8);
		(*header)["field_0xA"].get_to(m_header.field_0xA);
		(*header)["field_0xC"].get_to(m_header.field_0xC);
		(*header)["field_0xE"].get_to(m_header.field_0xE);
	}
	else {
		OUT_ERROR("Valid header is missing!");
		return FAILURE;
	}


	for(const auto& jChunk: j["chunks"]) {
		mpdfileChunk chunk;
		if (auto header = jChunk.find("header"); header != jChunk.end() && header->is_object()) {
			(*header)["map_offset"][0].get_to(chunk.info.header.map_offset_x);
			(*header)["map_offset"][1].get_to(chunk.info.header.map_offset_y);
			(*header)["map_offset"][2].get_to(chunk.info.header.map_offset_z);
			(*header)["map_offset"][3].get_to(chunk.info.header.map_offset_w);
			(*header)["rotation"][0].get_to(chunk.info.header.xRot);
			(*header)["rotation"][1].get_to(chunk.info.header.yRot);
			(*header)["rotation"][2].get_to(chunk.info.header.zRot);
			(*header)["rotation"][3].get_to(chunk.info.header.wRot);
			(*header)["map_offset2"][0].get_to(chunk.info.header.map_offset2_x);
			(*header)["map_offset2"][1].get_to(chunk.info.header.map_offset2_y);
			(*header)["map_offset2"][2].get_to(chunk.info.header.map_offset2_z);
			(*header)["number_of_tiles"].get_to(chunk.info.header.number_of_tiles);
			(*header)["unknown2"].get_to(chunk.info.header.unknown2);
			(*header)["index"].get_to(chunk.info.header.index);

			for (int i = 0; i < 14; ++i) {
				(*header)[std::string("unknown_data") + std::to_string(i + 1)].get_to(chunk.info.header.unknown_data2[i]);
			}
		}
		else {
			OUT_ERROR("Valid header for a chunk is missing!");
			return FAILURE;
		}

		auto& jTiles = jChunk["tiles"];
		for (const auto& jTile : jTiles) {
			chunk.tiles.push_back(jsonToTile(jTile));
		}
		int iObject = 0;
		std::vector<PG_MPDCustomModel> customModels;

		if(auto findIt = jChunk.find("objects"); findIt != jChunk.end()){
			const auto& jObjects = *findIt;
			for (const auto& jObject : jObjects) {
				//a name and no model id is a custom model, drawn from a resource and not from the table
				if (jObject.contains("name") && !jObject.contains("model_id")) {
					PG_MPDCustomModel& model = customModels.emplace_back();
					jObject["name"].get_to(model.name);
					model.object = mpdfileChunkObject::createNew();
					jObject["positon"][0].get_to(model.object.x);
					jObject["positon"][1].get_to(model.object.y);
					jObject["positon"][2].get_to(model.object.z);
					jObject["rotation"][0].get_to(model.object.xRot);
					jObject["rotation"][1].get_to(model.object.yRot);
					jObject["rotation"][2].get_to(model.object.zRot);
					jObject["scale"][0].get_to(model.object.xScale);
					jObject["scale"][1].get_to(model.object.yScale);
					jObject["scale"][2].get_to(model.object.zScale);
					continue;
				}

				if (iObject >= chunk.info.objects.size()) continue;
				auto& object = chunk.info.objects[iObject++];
				jObject["positon"][0].get_to(object.x);
				jObject["positon"][1].get_to(object.y);
				jObject["positon"][2].get_to(object.z);

				jObject["rotation"][0].get_to(object.xRot);
				jObject["rotation"][1].get_to(object.yRot);
				jObject["rotation"][2].get_to(object.zRot);

				jObject["scale"][0].get_to(object.xScale);
				jObject["scale"][1].get_to(object.yScale);
				jObject["scale"][2].get_to(object.zScale);


				jObject["animationFrame"].get_to(object.animationFrame);
				jObject["unk0x14"].get_to(object.unk0x14);
				jObject["model_id"].get_to(object.model_id);
				jObject["visible"].get_to(object.visible);
				jObject["unk0x17"].get_to(object.unk0x17);

				jObject["texture_scroll_speed"][0].get_to(object.textureScrollX);
				jObject["texture_scroll_speed"][1].get_to(object.textureScrollY);
				jObject["texture_scroll"][0].get_to(object.uvScrollX);
				jObject["texture_scroll"][1].get_to(object.uvScrollY);
			}
		}

		if (auto findIt = jChunk.find("eventTiles"); findIt != jChunk.end()) {
			const auto& jEventTiles = *findIt;
			iObject = 0;
			for (const auto& jEventTile : jEventTiles) {
				if (iObject >= chunk.info.eventTiles.size()) break;
				auto& eventTile = chunk.info.eventTiles[iObject++];
				jEventTile["x"].get_to(eventTile.x);
				jEventTile["z"].get_to(eventTile.z);
				jEventTile["index"].get_to(eventTile.index);
			}
			chunk.info.baseTile = jsonToTile(jChunk["baseTile"]);
		}
		m_chunks.push_back(std::move(chunk));
		setCustomModels(static_cast<int>(m_chunks.size()) - 1, std::move(customModels));
	}

	if (auto findIt = j.find("actors"); findIt != j.end()) {
		for (const auto& jActor : *findIt) {
			mpdfileActor actor;
			jActor["id"].get_to(actor.id);
			jActor["level"].get_to(actor.level);
			jActor["alliance"].get_to(actor.alliance);
			jActor["x"].get_to(actor.x);
			jActor["z"].get_to(actor.z);
			jActor["facing"].get_to(actor.facing);
			jActor["visible"].get_to(actor.visible);
			jActor["ai"].get_to(actor.ai);
			jActor["unknown3"].get_to(actor.unk5);
			jActor["unknown4"].get_to(actor.unk6);
			jActor["items"].get_to(actor.items);
			jActor["difficulty_spawn_rule"].get_to(actor.difficulty_spawn_rule);
			jActor["geo_effect"].get_to(actor.geo_effect);
			jActor["unk8"].get_to(actor.unk8);
			jActor["magic"].get_to(actor.magic);
			jActor["magic_level"].get_to(actor.magic_level);
			{
				for (int i = 0; i < 26; ++i) {
					jActor["unknown_stat" + std::to_string(i + 1)].get_to(actor.unk[i]);
				}
			}

			m_actors.push_back(std::move(actor));
		}
	}


	return SUCCESS;
}

void PG_MPD::clear() {
	m_chunks.clear();
	m_actors.clear();
	m_customMetaData.reset();
}

std::vector<PG_MPDCustomModel> PG_MPD::getCustomModels(int chunkIndex) const {
	return PG_MPDCustom::readModels(m_customMetaData, chunkIndex);
}

void PG_MPD::setCustomModels(int chunkIndex, std::vector<PG_MPDCustomModel>&& models) {
	PG_MPDCustom::writeModels(m_customMetaData, chunkIndex, models);
}

static inline nlohmann::json tileToJson(const mpdfileTile& tile) {
	nlohmann::json jTile;
	auto& textures = jTile["textures"];
	for (unsigned int t = 0; t < 12; t++) {
		nlohmann::json jTexture = {
			{"u",tile.textures[t].u },
			{"v",tile.textures[t].v },
			{"vRight",tile.textures[t].vRight },
			{"width",tile.textures[t].width },
			{"height",tile.textures[t].height },
			{"heightRight",tile.textures[t].heightRight },
			{"texture_index",tile.textures[t].textureIndex },
			{"transform",tile.textures[t].transform },
		};
		textures.push_back(std::move(jTexture));
	}
	{
		auto& corners = jTile["corners_top"];
		corners["SW"] = tile.cornersTop[0];
		corners["SE"] = tile.cornersTop[1];
		corners["NW"] = tile.cornersTop[2];
		corners["NE"] = tile.cornersTop[3];
	}
	{
		auto& corners = jTile["corners_sideN"];
		corners["a"] = tile.sideN.x;
		corners["b"] = tile.sideN.y;
	}
	{
		auto& corners = jTile["corners_sideE"];
		corners["a"] = tile.sideE.x;
		corners["b"] = tile.sideE.y;
	}
	{
		auto& corners = jTile["corners_sideS"];
		corners["a"] = tile.sideS.x;
		corners["b"] = tile.sideS.y;
	}
	{
		auto& corners = jTile["corners_sideW"];
		corners["a"] = tile.sideW.x;
		corners["b"] = tile.sideW.y;
	}

	{
		for (int i = 0; i < 4; ++i) {
			jTile["unknown_" + std::to_string(i + 1)] = tile.unk2[i];
		}
	}
	jTile["visible"] = tile.visible;
	jTile["x"] = tile.x;
	jTile["z"] = tile.z;
	{
		for (int i = 0; i < 4; ++i) {
			jTile["unknown_" + std::to_string(i + 1 + 4)] = tile.unk3[i];
		}
	}
	jTile["mobility"] = tile.mobility;
	jTile["geo_color"] = tile.geo_color;
	jTile["geo_mark"] = tile.geo_mark;
	{
		for (int i = 0; i < 6; ++i) {
			jTile["padding_" + std::to_string(i + 1)] = tile.padding[i];
		}
	}
	return jTile;
}

void PG_MPD::saveJson(const std::string& file) const {
	nlohmann::json j;
	j["header"] = {
		{"number_of_chunks", m_header.number_of_chunks},
		{"number_of_actors", m_header.number_of_actors},
		{"field_0x4", m_header.field_0x4 },
		{"field_0x6", m_header.field_0x6 },
		{"field_0x8", m_header.field_0x8 },
		{"field_0xA", m_header.field_0xA },
		{"field_0xC", m_header.field_0xC },
		{"field_0xE", m_header.field_0xE },
	};

	auto& chunks = j["chunks"];
	for (std::size_t chunkIndex = 0; chunkIndex < m_chunks.size(); ++chunkIndex) {
		const mpdfileChunk& chunk = m_chunks[chunkIndex];
		nlohmann::json jChunk;
		jChunk["header"] = {
			{"map_offset", {  chunk.info.header.map_offset_x, chunk.info.header.map_offset_y, chunk.info.header.map_offset_z, chunk.info.header.map_offset_w }},
			{"rotation", {  chunk.info.header.xRot, chunk.info.header.yRot, chunk.info.header.zRot, chunk.info.header.wRot }},
			{"map_offset2", {  chunk.info.header.map_offset2_x, chunk.info.header.map_offset2_y, chunk.info.header.map_offset2_z }},
			{"number_of_tiles", chunk.info.header.number_of_tiles},
			{"unknown2", chunk.info.header.unknown2},
			{"index", chunk.info.header.index},
		};
		for (int i = 0; i < 14; ++i) {
			jChunk["header"][std::string("unknown_data") + std::to_string(i + 1)] = chunk.info.header.unknown_data2[i];
		}

		auto& tiles = jChunk["tiles"];

		for (const mpdfileTile& tile : chunk.tiles) {
			tiles.push_back(tileToJson(tile));
		}

		auto& jObjects = jChunk["objects"];
		for (const auto& object : chunk.info.objects) {
			nlohmann::json jObject;

			jObject["positon"] = { object.x, object.y,object.z };
			jObject["rotation"] = { object.xRot, object.yRot, object.zRot };
			jObject["scale"] = { object.xScale, object.yScale, object.zScale };
			jObject["animationFrame"] = object.animationFrame;
			jObject["unk0x14"] = object.unk0x14;
			jObject["model_id"] = object.model_id;
			jObject["visible"] = object.visible;
			jObject["unk0x17"] = object.unk0x17;
			jObject["texture_scroll_speed"] = { object.textureScrollX, object.textureScrollY };
			jObject["texture_scroll"] = { object.uvScrollX, object.uvScrollY };

			jObjects.push_back(std::move(jObject));
		}
		//no model_id, which is what marks them when read back
		for (const PG_MPDCustomModel& model : getCustomModels(static_cast<int>(chunkIndex))) {
			nlohmann::json jObject;
			jObject["name"] = model.name;
			jObject["positon"] = { model.object.x, model.object.y, model.object.z };
			jObject["rotation"] = { model.object.xRot, model.object.yRot, model.object.zRot };
			jObject["scale"] = { model.object.xScale, model.object.yScale, model.object.zScale };
			jObjects.push_back(std::move(jObject));
		}


		auto& jEventTiles = jChunk["eventTiles"];
		for (const auto& eventTile : chunk.info.eventTiles) {
			nlohmann::json jEventTile;
			jEventTile["x"] = eventTile.x;
			jEventTile["z"] = eventTile.z;
			jEventTile["index"] = eventTile.index;
			jEventTiles.push_back(std::move(jEventTile));
		}
		jChunk["baseTile"] = tileToJson(chunk.info.baseTile);

		chunks.push_back(std::move(jChunk));
	}

	auto& actors = j["actors"];
	for (const mpdfileActor& actor : m_actors) {
		nlohmann::json jActor;
		jActor["id"] = actor.id;
		jActor["level"] = actor.level;
		jActor["alliance"] = actor.alliance;
		jActor["x"] = actor.x;
		jActor["z"] = actor.z;
		jActor["facing"] = actor.facing;
		jActor["visible"] = actor.visible;
		jActor["ai"] = actor.ai;
		jActor["unknown3"] = actor.unk5;
		jActor["unknown4"] = actor.unk6;
		jActor["items"] = actor.items;
		jActor["difficulty_spawn_rule"] = actor.difficulty_spawn_rule;
		jActor["geo_effect"] = actor.geo_effect;
		jActor["unk8"] = actor.unk8;
		jActor["magic"] = actor.magic;
		jActor["magic_level"] = actor.magic_level;
		{
			for (int i = 0; i < 26; ++i) {
				jActor["unknown_stat" + std::to_string(i + 1)] = actor.unk[i];
			}
		}

		actors.push_back(std::move(jActor));
	}

	std::ofstream myfile;
	myfile.open(file.c_str());
	myfile << j.dump(4);
	myfile.close();
}


bool PG_MPD::save(const std::string& file) {
	return save(PG_File(file));
}

bool PG_MPD::save(const PG_File& file) {
	try {
		PG::STREAM::OutByteFile writer(file);
		if(!writer.isOpen()) return FAILURE;
		const bool result = save(&writer);
		writer.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to write MPD! : " << e.what());
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to write MPD!");
		return FAILURE;
	}

	return SUCCESS;
}

bool PG_MPD::save(PG::STREAM::Out* writer) {
	if (!writer) {
		OUT_ERROR("Writer is nullptr!");
		return FAILURE;
	}
	if (m_chunks.empty() && m_actors.empty()) {
		OUT_ERROR("Nothing to save!");
		return FAILURE;
	}

	const unsigned int start = writer->pos();
	writer->write((char*)&m_header, sizeof(mpdfileHeader));
	for (std::size_t i = 0; i < m_chunks.size(); ++i) {
		const mpdfileChunk& chunk = m_chunks[i];
		const std::vector<PG_MPDCustomModel> customModels = getCustomModels(static_cast<int>(i));
		if (customModels.empty()) {
			writer->write((char*)&chunk.info, sizeof(mpdfileChunkDef));
			continue;
		}
		mpdfileChunkDef info = chunk.info;
		auto freeIt = std::ranges::find_if(info.objects, &mpdfileChunkObject::isEmpty);
		if (freeIt == info.objects.end()) {
			OUT_WARNING("Chunk " << i << " has no free object slot for the custom models marker, it replaces the last object!");
			freeIt = std::prev(info.objects.end());
		}
		*freeIt = mpdfileChunkObject::createCustomModelsMarker(customModels.front().object);
		writer->write((char*)&info, sizeof(mpdfileChunkDef));
	}

	for (const auto& chunk : m_chunks) {
		writer->write((char*)chunk.tiles.data(), sizeof(mpdfileTile) * chunk.tiles.size());
	}

	writer->write((char*)m_actors.data(), sizeof(mpdfileActor) * m_actors.size());

	//isSet and not isEmpty, a block whose keys were all removed still has its root and nothing to write
	if (m_customMetaData.isSet()) {
		const unsigned int blockStart = writer->pos() - start;
		writer->writeString("PGMD");
		m_customMetaData.save(writer);
		writer->writeInt(static_cast<int>(blockStart)); //so the block can be found from the end
	}

	return SUCCESS;
}

mpdfileHeader& PG_MPD::getHeader() {
	return m_header;
}
const mpdfileHeader& PG_MPD::getHeader() const {
	return m_header;
}

std::vector<mpdfileChunk>& PG_MPD::getChunks() {
	return m_chunks;
}
const std::vector<mpdfileChunk>& PG_MPD::getChunks() const {
	return m_chunks;
}

std::vector<mpdfileActor>& PG_MPD::getActors() {
	return m_actors;
}
const std::vector<mpdfileActor>& PG_MPD::getActors() const {
	return m_actors;
}
