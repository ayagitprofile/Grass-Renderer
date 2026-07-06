#pragma once

#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "Types.h"

namespace Grass {
class Model
{
public:
	Model(Ref<Mesh> mesh, Ref<Material> material) : Mesh(mesh), Material(material) {};
	Model(){};

	Transform Transform;
	Ref<Material> Material;
	Ref<Mesh> Mesh;
};
}