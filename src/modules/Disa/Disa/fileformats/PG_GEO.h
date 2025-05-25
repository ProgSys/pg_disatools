
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

#pragma once

#include <string>
#include <vector>
#include <array>
#include <ostream>
#include <glm/glm.hpp>
#include <Util/Typedefs.h>
#include <Util/PG_Image.h>
#include <Util/PG_File.h>
#include <Util/Volume/PG_AABB.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamOut.h>
#include <Disa/fileformats/PG_TX2.h>

PACK(struct geofileHeader// x bytes
{
    int unk1, number_of_nodes, unk3, number_of_transformations;
    int number_of_keyframes, offset_to_first_mesh, number_of_meshes, unk6;
});

PACK(struct geofileTransformation// 64 bytes
{
    glm::vec3 position; float unk7;
    glm::vec3 rotation; float unk8;
    glm::vec3 scale; float unk9;
    glm::vec3 times;  float unk13;
});

PACK(struct geofileKeyframe// 4 bytes
{
    unsigned short value, time;
});

PACK(struct geofileTreeNode// 14 bytes
{
    short parent_index, transformation_index, number_of_transformations, model_index, transformation_mode, keyframe_index, number_of_keyframes;
    //transformation_mode: 2 or 3 - Billboard
});

PACK(struct geofileMeshHeader // x bytes
{
    unsigned short number_of_triangles, isNew;
    int unk9, number_of_surfaces, number_of_textures;
    int start_offset_texture, end_offset_mesh, size_of_triangles, unk12; //origin is from geofileSurface location
});

PACK(struct geofileSurface// x bytes
{
    unsigned char rMult, gMult, bMult, aMult; //RGBA color texture multiplication
    unsigned short index, length; // "length" is the number of vertices inside group
    int unk4;//somekind of texture effects?
    unsigned char additive, animated, unk5, unk6; //somekind of texture effects?
});

PACK(struct geofileTextureHeader// x bytes
{
    unsigned short totalSize;
    unsigned char unk0;
    unsigned char type : 7;
    bool unkFlag : 1;
    unsigned short width, height, colortableSize;
    unsigned char powerWidth, powerHeight;
    unsigned int unk4;
});

struct geofileTexture {
    geofileTextureHeader header;
    std::vector<glm::rgba> colortable;
    std::vector<char> pixels;
    PG_TX2::Type type; //helper variable to track the type (is not stored)

    /**
    * @brief Sets header to the given type
    */
    void setTypeArgs(PG_TX2::Type type);
    /**
    * @returns The total byte size of texture. Including header and all.
    * The short in the header is way to small, so we need to compute it
    */
    unsigned int getTotalByteSize() const;

    PG_RGBAImage toImageRGBA() const;

};

struct geofileMesh {
    geofileMeshHeader header;

    std::vector<geofileSurface> surfaces;
    //buffers
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> tangent;
    std::vector<glm::vec3> bitangent;
    std::vector<glm::vec2> uv;
    std::vector<geofileTexture> textures;
};

class PG_GEO {
public:


    bool open(const std::string& file);
    bool open(const PG_File& file);
    bool open(PG::STREAM::In* reader);

    bool save(const PG_File& file) const;
    bool save(PG::STREAM::Out* writer) const;

    bool importGLTF(const PG_File& file);

    void clear();

    void dump(const std::string& file) const;

    inline geofileHeader& getHeader() { return m_header; }
    inline const geofileHeader& getHeader() const { return m_header; }

    inline std::vector<geofileTransformation>& getTransformations() { return m_transformations; }
    inline const std::vector<geofileTransformation>& getTransformations() const { return m_transformations; }

    inline std::vector<geofileKeyframe>& getKeyframes() { return m_keyframes; }
    inline const std::vector<geofileKeyframe>& getKeyframes() const { return m_keyframes; }

    inline std::vector<geofileTreeNode>& getTreeNodes() { return m_treeNodes; }
    inline const std::vector<geofileTreeNode>& getTreeNodes() const { return m_treeNodes; }

    inline std::vector<geofileMesh>& getModels() { return m_models; }
    inline const std::vector<geofileMesh>& getModels() const { return m_models; }

    PG_AABB computeAABB() const;
    /**
    * @brief Recompute all the offset addresses for the given data.
    */
    void computeAddresses();

private:
    geofileHeader m_header;
	std::vector<geofileTransformation> m_transformations; //must have at least one
	std::vector<geofileKeyframe> m_keyframes; 
	std::vector<geofileTreeNode> m_treeNodes;
    std::vector<geofileMesh> m_models;
};

