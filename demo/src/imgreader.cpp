#define STB_IMAGE_IMPLEMENTATION
#include "imgreader.h"

void imageFileToRgba(const char *filename, int *img_width, int *img_height, uint8_t **pixels)
{
    int img_channels;
    stbi_set_flip_vertically_on_load(true);
    *pixels = stbi_load(filename, img_width, img_height, &img_channels, STBI_rgb_alpha);
    if (*pixels == NULL)
    {
        fprintf(stderr, "[imgreader] Error: could not read %s into RGBA image\n", filename);
    }
}

void freeRgba(uint8_t *pixels)
{
    stbi_image_free(pixels);
}
