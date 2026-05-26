#include "image_io.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool load_image_rgba(const std::string& path, std::vector<uint8_t>& rgba_pixels, int& width, int& height)
{
    int channels = 0;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        return false;
    }

    const int bytes = width * height * 4;
    rgba_pixels.assign(pixels, pixels + bytes);
    stbi_image_free(pixels);
    return true;
}

bool write_image_gray_png(const std::string& path, const uint8_t* gray_pixels, int width, int height)
{
    const int stride = width;
    return stbi_write_png(path.c_str(), width, height, STBI_grey, gray_pixels, stride) != 0;
}
