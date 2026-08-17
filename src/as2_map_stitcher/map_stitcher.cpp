#include "map_stitcher.h"

#include "bmp_reader.h"
#include "png_writer.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

struct Tile {
    fs::path path;
    int x = 0;
    int y = 0;
};

using MapTiles = std::map<std::string, std::vector<Tile>>;

MapTiles find_map_tiles(const fs::path& input_directory) {
    const std::regex file_pattern(
        R"(^(.+)_([0-9]+)_([0-9]+)\.bmp$)",
        std::regex::icase);
    MapTiles maps;

    for (const auto& entry : fs::directory_iterator(input_directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string filename = entry.path().filename().string();
        std::smatch match;

        if (!std::regex_match(filename, match, file_pattern)) {
            continue;
        }

        maps[match[1]].push_back({
            entry.path(),
            std::stoi(match[2]),
            std::stoi(match[3])});
    }

    return maps;
}

Image create_empty_image(int width, int height) {
    const std::size_t pixel_count =
        static_cast<std::size_t>(width) * height;
    return Image{
        width,
        height,
        std::vector<std::uint8_t>(pixel_count * 3)};
}

void validate_tile_coordinates(const std::vector<Tile>& tiles) {
    for (const Tile& tile : tiles) {
        if (tile.x < 0 || tile.y < 0) {
            throw std::runtime_error("negative tile coordinate");
        }
    }
}

Image stitch_one_map(const std::string& map_name,
                     const std::vector<Tile>& tiles) {
    const Image first_tile = read_bmp(tiles.front().path);
    int max_x = 0;
    int max_y = 0;

    for (const Tile& tile : tiles) {
        max_x = std::max(max_x, tile.x);
        max_y = std::max(max_y, tile.y);
    }

    if (max_x > (std::numeric_limits<int>::max() / first_tile.width) - 1 ||
        max_y > (std::numeric_limits<int>::max() / first_tile.height) - 1) {
        throw std::runtime_error("stitched image is too large");
    }

    const int full_width = (max_x + 1) * first_tile.width;
    const int full_height = (max_y + 1) * first_tile.height;
    Image full_image = create_empty_image(full_width, full_height);

    for (const Tile& tile : tiles) {
        const Image part = read_bmp(tile.path);

        if (part.width != first_tile.width ||
            part.height != first_tile.height) {
            throw std::runtime_error(
                "tile sizes do not match in " + map_name);
        }

        for (int y = 0; y < part.height; ++y) {
            const std::size_t source_offset =
                static_cast<std::size_t>(y) * part.width * 3;
            const std::size_t destination_y =
                static_cast<std::size_t>(tile.y * first_tile.height + y);
            const std::size_t destination_x =
                static_cast<std::size_t>(tile.x * first_tile.width);
            const std::size_t destination_offset =
                (destination_y * full_image.width + destination_x) * 3;

            std::copy_n(part.rgb.data() + source_offset,
                        part.width * 3,
                        full_image.rgb.data() + destination_offset);
        }
    }

    return full_image;
}

}  // namespace

StitchResult stitch_maps(const std::filesystem::path& input_directory,
                         const std::filesystem::path& output_directory) {
    const MapTiles maps = find_map_tiles(input_directory);

    if (maps.empty()) {
        throw std::runtime_error(
            "no files matching <map>_<x>_<y>.bmp were found");
    }

    StitchResult result;
    result.map_count = static_cast<int>(maps.size());

    for (const auto& [map_name, tiles] : maps) {
        validate_tile_coordinates(tiles);

        for (const Tile& tile : tiles) {
            result.source_tiles.push_back(tile.path);
        }

        const Image image = stitch_one_map(map_name, tiles);
        const auto output_path = output_directory / (map_name + "_full.png");
        write_png(output_path, image);
        std::cout << "Created PNG: " << output_path << '\n';
    }

    return result;
}
