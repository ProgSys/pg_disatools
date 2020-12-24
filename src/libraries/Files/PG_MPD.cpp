/*
 * PG_MPD.cpp
 *
 *  Created on: 09.09.2016
 *      Author: ProgSys
 */

#include <Files/PG_MPD.h>
#include <algorithm>
#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>

#include <Util/PG_Image.h>
#include <Files/PG_ImageFiles.h>

namespace PG {
	namespace FILE {

		MPD::MPD() {
			// TODO Auto-generated constructor stub

		}

		MPD::~MPD() {
			// TODO Auto-generated destructor stub
		}

		// functions
		bool MPD::open(const std::string& file) {
			if (file.empty()) {
				PG_ERROR_STREAM("Given filename is empty!");
				return FAILURE;
			}

			return open(PG::UTIL::File(file));
		}

		bool MPD::open(const PG::UTIL::File& file) {
			clear();
			if (!file.exists()) {
				PG_ERROR_STREAM("Target file doesn't exist!");
				return FAILURE;
			}



			try {
				PG::STREAM::InByteFile reader(file);
				if (!reader.isopen()) {
					PG_ERROR_STREAM("Coudn't open file!");
					return FAILURE;
				}

				const unsigned int file_size = reader.size();
				if (file_size < 32) {
					clear();
					PG_ERROR_STREAM("Given file is too small!");
					return FAILURE;
				}

				reader.read((char*)&m_header, sizeof(mpdfileHeader));

				if (m_header.number_of_chunks == 0) {
					clear();
					PG_ERROR_STREAM("The map is empty!");
					return FAILURE;
				}
				if (m_header.number_of_chunks > 900) {
					clear();
					PG_ERROR_STREAM("The map has too many chunks!");
					return FAILURE;
				}

				if (m_header.number_of_actors > 900) {
					clear();
					PG_ERROR_STREAM("The map has too many actors!");
					return FAILURE;
				}

				for (unsigned int i = 0; i < m_header.number_of_chunks; i++) {
					mpdfileChunk chunk;
					reader.read((char*)&chunk.info, sizeof(mpdfileChunkDef));

					if (chunk.info.header.number_of_tiles > 900) {
						clear();
						PG_ERROR_STREAM("The chunk " << i << " has too many tiles!");
						return FAILURE;
					}

					chunk.tiles.reserve(chunk.info.header.number_of_tiles);
					for (unsigned int a = 0; a < chunk.info.header.number_of_tiles; a++) {
						mpdfileTile tile;
						reader.read((char*)&tile, sizeof(mpdfileTile));
						chunk.tiles.push_back(tile);
					}
					m_chunks.push_back(chunk);
				}

				m_actors.reserve(m_header.number_of_actors);
				for (unsigned int i = 0; i < m_header.number_of_actors; i++) {
					mpdfileActor actor;
					reader.read((char*)&actor, sizeof(mpdfileActor));
					m_actors.push_back(actor);
				}
				reader.close();

			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM("Failed to import SH! : " << e.what());
				clear();
				return FAILURE;
			}
			catch (...) {
				PG_ERROR_STREAM("Failed to import SH!");
				clear();
				return FAILURE;
			}

			return SUCCESS;
		}

		void MPD::clear() {
			m_chunks.clear();
			m_actors.clear();
		}

		void MPD::dump(const std::string& file) const {
			std::ofstream myfile;

			myfile.open(file.c_str());

			myfile << "Header:\n";
			myfile << " * number_of_chunks:" << m_header.number_of_chunks << "\n";
			myfile << " * number_of_actors:" << m_header.number_of_actors << "\n";
			myfile << " * unknown1:" << m_header.unknown1 << "\n";
			myfile << " * unknown2:" << m_header.unknown2 << "\n";
			myfile << " * unknown3:" << m_header.unknown3 << "\n";
			myfile << " * unknown4:" << m_header.unknown4 << "\n";
			myfile << " * unknown5:" << m_header.unknown5 << "\n";
			myfile << " * unknown6:" << m_header.unknown6 << "\n";
			myfile << "Chunks:\n";

			for (const mpdfileChunk& chunk : m_chunks) {
				myfile << "  Chunk Header:\n";
				myfile << "    * map_offset_x: " << chunk.info.header.map_offset_x << "\n";
				myfile << "    * map_offset_y: " << chunk.info.header.map_offset_y << "\n";
				myfile << "    * map_offset_z: " << chunk.info.header.map_offset_z << "\n";
				myfile << "    * number_of_tiles: " << chunk.info.header.number_of_tiles << "\n";
				myfile << "    * index: " << chunk.info.header.index << "\n";
				myfile << "  Chunk objects:\n";
				myfile << "    * unk1: " << chunk.info.objects[0].unk1 << "\n";
				myfile << "    * unk2: " << chunk.info.objects[0].unk2 << "\n";
				/*
				myfile << "  Chunk event tiles:\n";
				for(unsigned int i = 0; i < 16; i++){
					myfile << "    Event tile:\n";
					myfile << "      * index: "<<chunk.info.eventTiles[i].index<<"\n";
					myfile << "      * x: "<<(int)chunk.info.eventTiles[i].x<<"\n";
					myfile << "      * z: "<<(int)chunk.info.eventTiles[i].z<<"\n";
				}
	  */

				myfile << "      Tiles (" << chunk.tiles.size() << "):\n";



				for (const mpdfileTile& tile : chunk.tiles) {
					myfile << "        Tile:\n";
					for (unsigned int t = 9; t < 10; t++) {
						myfile << "        Texture" << t << ":\n";
						myfile << "           * u: " << (int)tile.textures[t].u << "\n";
						myfile << "           * v: " << (int)tile.textures[t].v << "\n";
						myfile << "           * w: " << (int)tile.textures[t].width << "\n";
						myfile << "           * h: " << (int)tile.textures[t].height << "\n";
						myfile << "           * t: " << (int)tile.textures[t].textureIndex << "\n";
						myfile << "           * m: " << (int)tile.textures[t].mirror << "\n";
					}
					/*
					myfile << "           * x: "<<(int)tile.x<<"\n";
					myfile << "           * z: "<<(int)tile.z<<"\n";
					myfile << "           * CA: "<<(int)tile.corners[0]<<"\n";
					myfile << "           * CB: "<<(int)tile.corners[1]<<"\n";
					myfile << "           * CC: "<<(int)tile.corners[2]<<"\n";
					myfile << "           * CD: "<<(int)tile.corners[3]<<"\n";
					*/
				}
			}

			myfile.close();
		}


		mpdfileHeader& MPD::getHeader() {
			return m_header;
		}
		const mpdfileHeader& MPD::getHeader() const {
			return m_header;
		}

		std::vector<mpdfileChunk>& MPD::getChunks() {
			return m_chunks;
		}
		const std::vector<mpdfileChunk>& MPD::getChunks() const {
			return m_chunks;
		}

		std::vector<mpdfileActor>& MPD::getActors() {
			return m_actors;
		}
		const std::vector<mpdfileActor>& MPD::getActors() const {
			return m_actors;
		}
	} /* namespace FILE */
} /* namespace PG */
