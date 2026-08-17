#pragma once

#include "image.h"

#include <filesystem>

Image read_bmp(const std::filesystem::path& path);
