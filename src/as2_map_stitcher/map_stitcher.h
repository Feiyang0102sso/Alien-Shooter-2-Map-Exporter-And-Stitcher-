#pragma once

#include <filesystem>
#include <vector>

struct StitchResult {
    int map_count = 0;
    std::vector<std::filesystem::path> source_tiles;
};

StitchResult stitch_maps(const std::filesystem::path& input_directory,
                         const std::filesystem::path& output_directory);
