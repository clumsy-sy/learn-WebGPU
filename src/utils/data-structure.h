#pragma once

#include "global.h"

namespace webgpu {

// 定义 window_size_t 结构体
struct window_size_t {
    uint32_t width;
    uint32_t height;
    // 别名 x 和 y 的 getter 函数
    uint32_t& x() {
        return width;
    }

    uint32_t& y() {
        return height;
    }
};


// 定义 Uniform 结构体

struct Uniform {
    glm::mat4x4 projectionMatrix = {};
    glm::mat4x4 viewMatrix = {};
    glm::mat4x4 modelMatrix = {};
    std::array<float, 4> color;
    float time;
    float _pad[3];
};

/**
 * 一个描述顶点缓冲区中数据布局的结构体
 * 不会实例化 ，但是会对它使用 `sizeof`（获取大小） 和 `offsetof`（获取偏移）
 */
struct VertexAttributes {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

// 使用编译器检查确保 Uniform 结构体大小为16的倍数
static_assert(sizeof(Uniform) % 16 == 0);


bool loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData);

}