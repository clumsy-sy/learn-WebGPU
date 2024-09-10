#include "data-structure.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny-obj-loader.h"

namespace webgpu {


 std::ostream& operator<<(std::ostream& os, const Uniform& uniform){
    os << "projectionMatrix:\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            os << uniform.projectionMatrix[i][j] << ' ';
        }
        os << '\n';
    }

    os << "\nviewMatrix:\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            os << uniform.viewMatrix[i][j] << ' ';
        }
        os << '\n';
    }

    os << "\nmodelMatrix:\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            os << uniform.modelMatrix[i][j] << ' ';
        }
        os << '\n';
    }

    os << "\ncolor: ";
    for (auto& c : uniform.color) {
        os << c << ' ';
    }

    os << "\ntime: " << uniform.time;

    return os;
}

bool loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

	// Check errors
	if (!warn.empty()) {
		std::cout << "[LoadObj]" << warn << '\n';
	}

	if (!err.empty()) {
		std::cerr << "[LoadObj]" << err << '\n';
	}

	if (!ret) {
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto& shape : shapes) {
		size_t offset = vertexData.size();
		printf("offest = %zu\n", offset);
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
			const tinyobj::index_t& idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
				attrib.vertices[3 * idx.vertex_index + 0],
				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
				attrib.vertices[3 * idx.vertex_index + 1]
			};

			// Also apply the transform to normals!!
			vertexData[offset + i].normal = {
				attrib.normals[3 * idx.normal_index + 0],
				-attrib.normals[3 * idx.normal_index + 2],
				attrib.normals[3 * idx.normal_index + 1]
			};

			vertexData[offset + i].color = {
				attrib.colors[3 * idx.vertex_index + 0],
				attrib.colors[3 * idx.vertex_index + 1],
				attrib.colors[3 * idx.vertex_index + 2]
			};
		}
	}
	LOG("Load obj finish, vertexData size = %zu\n", vertexData.size());
	return true;
}

}