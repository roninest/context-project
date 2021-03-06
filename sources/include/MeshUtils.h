//MIT License
//
//Copyright (c) 2021 Andrei Vasilev
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <OgreMesh.h>
#include <string>

namespace Ogre {
class VertexDeclaration;
class Entity;
}

namespace xio {

bool HasNoTangentsAndCanGenerate(Ogre::VertexDeclaration *vertex_declaration);

void EnsureHasTangents(Ogre::MeshPtr mesh);

void UpdateMeshEdgeList(Ogre::Entity *entity);

void UpdateMeshMaterial(Ogre::MeshPtr mesh,
						bool cast_shadows = true,
						const std::string &material_name = "",
						bool planar_reflection = false,
						bool active_ibl = false);

void UpdateMeshMaterial(const std::string &mesh_name,
						bool cast_shadows = true,
						const std::string &material_name = "",
						bool planar_reflection = false,
						bool active_ibl = false);

void UpdateEntityMaterial(Ogre::Entity *entity,
						  bool cast_shadows = true,
						  const std::string &material_name = "",
						  bool planar_reflection = false,
						  bool active_ibl = false);

}