#pragma once

#include <cstdint>
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

}