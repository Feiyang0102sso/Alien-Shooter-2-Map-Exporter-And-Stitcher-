#pragma once

#include "image.h"

#include <filesystem>

void write_png(const std::filesystem::path& path, const Image& image);
