#include "bmp_reader.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

std::uint16_t read_u16(const std::vector<std::uint8_t>& bytes,
                       std::size_t position) {
    return static_cast<std::uint16_t>(
        bytes[position] | (bytes[position + 1] << 8));
}

std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes,
                       std::size_t position) {
    return static_cast<std::uint32_t>(
        bytes[position] |
        (bytes[position + 1] << 8) |
        (bytes[position + 2] << 16) |
        (bytes[position + 3] << 24));
}

}  // namespace

Image read_bmp(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open " + path.string());
    }

    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    if (bytes.size() < 54 || bytes[0] != 'B' || bytes[1] != 'M') {
        throw std::runtime_error("not a BMP file");
    }

    const std::uint32_t pixel_offset = read_u32(bytes, 10);
    const std::uint32_t dib_header_size = read_u32(bytes, 14);

    if (dib_header_size < 40 || bytes.size() < 14 + dib_header_size) {
        throw std::runtime_error("unsupported BMP header");
    }

    const auto width = static_cast<std::int32_t>(read_u32(bytes, 18));
    const auto signed_height =
        static_cast<std::int32_t>(read_u32(bytes, 22));
    const std::uint16_t planes = read_u16(bytes, 26);
    const std::uint16_t bits_per_pixel = read_u16(bytes, 28);
    const std::uint32_t compression = read_u32(bytes, 30);

    const bool supported_format =
        width > 0 &&
        signed_height != 0 &&
        planes == 1 &&
        (bits_per_pixel == 24 || bits_per_pixel == 32) &&
        compression == 0;

    if (!supported_format) {
        throw std::runtime_error(
            "only uncompressed 24/32-bit BMP is supported");
    }

    const int height = signed_height < 0 ? -signed_height : signed_height;
    const std::size_t bytes_per_pixel = bits_per_pixel / 8;
    const std::size_t row_stride =
        ((static_cast<std::size_t>(width) * bytes_per_pixel + 3) / 4) * 4;

    const bool data_fits_file =
        pixel_offset <= bytes.size() &&
        row_stride <= std::numeric_limits<std::size_t>::max() / height &&
        pixel_offset + row_stride * height <= bytes.size();

    if (!data_fits_file) {
        throw std::runtime_error("truncated BMP pixel data");
    }

    const std::size_t rgb_size =
        static_cast<std::size_t>(width) * height * 3;
    Image image{width, height, std::vector<std::uint8_t>(rgb_size)};

    for (int y = 0; y < height; ++y) {
        const int source_y = signed_height < 0 ? y : height - 1 - y;
        const auto* source = bytes.data() +
            pixel_offset + static_cast<std::size_t>(source_y) * row_stride;
        auto* destination = image.rgb.data() +
            static_cast<std::size_t>(y) * width * 3;

        for (int x = 0; x < width; ++x) {
            const std::size_t source_offset =
                static_cast<std::size_t>(x) * bytes_per_pixel;
            const std::size_t destination_offset =
                static_cast<std::size_t>(x) * 3;

            // BMP stores colors as B, G, R. The image uses R, G, B.
            destination[destination_offset] = source[source_offset + 2];
            destination[destination_offset + 1] = source[source_offset + 1];
            destination[destination_offset + 2] = source[source_offset];
        }
    }

    return image;
}
