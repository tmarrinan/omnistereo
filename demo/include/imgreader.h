#include "stb_image.h"

void imageFileToRgba(const char *filename, int *img_width, int *img_height, uint8_t **pixels);
void freeRgba(uint8_t *pixels);
