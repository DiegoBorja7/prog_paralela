#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <cstdint>
#include <string>
#include <vector>

bool load_image_rgba(const std::string& path, std::vector<uint8_t>& rgba_pixels, int& width, int& height);
bool write_image_gray_png(const std::string& path, const uint8_t* gray_pixels, int width, int height);

#endif
