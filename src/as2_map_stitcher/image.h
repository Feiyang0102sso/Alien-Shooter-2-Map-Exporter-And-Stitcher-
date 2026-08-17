#pragma once

#include <cstdint>
#include <vector>

struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgb;
};
