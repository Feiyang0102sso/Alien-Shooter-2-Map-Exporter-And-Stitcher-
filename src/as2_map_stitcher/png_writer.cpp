#include "png_writer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include <zlib.h>

namespace {

std::uint32_t crc32(const std::uint8_t* data, std::size_t size) {
    std::uint32_t crc = 0xffffffffu;

    for (std::size_t index = 0; index < size; ++index) {
        crc ^= data[index];

        for (int bit = 0; bit < 8; ++bit) {
            const std::uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }

    return ~crc;
}

void append_big_endian_u32(std::vector<std::uint8_t>& output,
                           std::uint32_t value) {
    output.push_back(static_cast<std::uint8_t>(value >> 24));
    output.push_back(static_cast<std::uint8_t>(value >> 16));
    output.push_back(static_cast<std::uint8_t>(value >> 8));
    output.push_back(static_cast<std::uint8_t>(value));
}

void append_chunk(std::vector<std::uint8_t>& png,
                  const char type[4],
                  const std::vector<std::uint8_t>& data) {
    append_big_endian_u32(png, static_cast<std::uint32_t>(data.size()));

    const std::size_t chunk_start = png.size();
    png.insert(png.end(), type, type + 4);
    png.insert(png.end(), data.begin(), data.end());

    const std::size_t chunk_size = 4 + data.size();
    append_big_endian_u32(png, crc32(png.data() + chunk_start, chunk_size));
}

std::uint8_t paeth_predictor(std::uint8_t left,
                             std::uint8_t above,
                             std::uint8_t upper_left) {
    const int estimate = static_cast<int>(left) + above - upper_left;
    const int distance_left = std::abs(estimate - left);
    const int distance_above = std::abs(estimate - above);
    const int distance_upper_left = std::abs(estimate - upper_left);

    if (distance_left <= distance_above &&
        distance_left <= distance_upper_left) {
        return left;
    }

    if (distance_above <= distance_upper_left) {
        return above;
    }

    return upper_left;
}

std::uint64_t filter_score(const std::vector<std::uint8_t>& row) {
    std::uint64_t score = 0;

    for (const std::uint8_t value : row) {
        const int signed_value = value < 128 ? value : value - 256;
        score += static_cast<std::uint64_t>(std::abs(signed_value));
    }

    return score;
}

std::vector<std::uint8_t> make_scanlines(const Image& image) {
    std::vector<std::uint8_t> scanlines;
    const std::size_t row_size = static_cast<std::size_t>(image.width) * 3;
    scanlines.reserve(static_cast<std::size_t>(image.height) * (row_size + 1));

    std::vector<std::uint8_t> previous_row(row_size, 0);

    for (int y = 0; y < image.height; ++y) {
        const auto row_start = image.rgb.begin() + static_cast<std::size_t>(y) * row_size;
        const std::vector<std::uint8_t> current_row(row_start, row_start + row_size);
        std::vector<std::uint8_t> best_row = current_row;
        std::uint8_t best_filter = 0;
        std::uint64_t best_score = filter_score(best_row);

        for (std::uint8_t filter = 1; filter <= 4; ++filter) {
            std::vector<std::uint8_t> filtered_row(row_size);

            for (std::size_t index = 0; index < row_size; ++index) {
                const std::uint8_t current = current_row[index];
                const std::uint8_t left = index >= 3 ? current_row[index - 3] : 0;
                const std::uint8_t above = previous_row[index];
                const std::uint8_t upper_left = index >= 3 ? previous_row[index - 3] : 0;
                std::uint8_t prediction = 0;

                if (filter == 1) {
                    prediction = left;
                } else if (filter == 2) {
                    prediction = above;
                } else if (filter == 3) {
                    prediction = static_cast<std::uint8_t>((left + above) / 2);
                } else {
                    prediction = paeth_predictor(left, above, upper_left);
                }

                filtered_row[index] = current - prediction;
            }

            const std::uint64_t score = filter_score(filtered_row);
            if (score < best_score) {
                best_score = score;
                best_filter = filter;
                best_row = std::move(filtered_row);
            }
        }

        scanlines.push_back(best_filter);
        scanlines.insert(scanlines.end(), best_row.begin(), best_row.end());
        previous_row = current_row;
    }

    return scanlines;
}

std::vector<std::uint8_t> compress_scanlines(
    const std::vector<std::uint8_t>& scanlines) {
    if (scanlines.size() > std::numeric_limits<uLong>::max()) {
        throw std::runtime_error("image data is too large for zlib");
    }

    const uLong source_size = static_cast<uLong>(scanlines.size());
    uLong compressed_size = compressBound(source_size);
    std::vector<std::uint8_t> compressed(compressed_size);

    const int result = compress2(
        compressed.data(),
        &compressed_size,
        scanlines.data(),
        source_size,
        Z_BEST_COMPRESSION);

    if (result != Z_OK) {
        throw std::runtime_error("PNG compression failed");
    }

    compressed.resize(compressed_size);
    return compressed;
}

}  // namespace

void write_png(const std::filesystem::path& path, const Image& image) {
    const auto scanlines = make_scanlines(image);
    const auto compressed_data = compress_scanlines(scanlines);

    std::vector<std::uint8_t> png{
        137, 80, 78, 71, 13, 10, 26, 10};

    std::vector<std::uint8_t> header;
    append_big_endian_u32(header, image.width);
    append_big_endian_u32(header, image.height);
    header.insert(header.end(), {8, 2, 0, 0, 0});

    append_chunk(png, "IHDR", header);
    append_chunk(png, "IDAT", compressed_data);
    append_chunk(png, "IEND", {});

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot create " + path.string());
    }

    file.write(reinterpret_cast<const char*>(png.data()),
               static_cast<std::streamsize>(png.size()));
}
