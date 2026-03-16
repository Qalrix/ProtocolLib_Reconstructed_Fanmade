// bitmap_tool.h
#ifndef BITMAP_TOOL_H
#define BITMAP_TOOL_H

#include <stdint.h>
#include <stdbool.h>

// Bitmap formats
typedef enum {
    BMP_FORMAT_RGB565 = 0x01,
    BMP_FORMAT_RGB888 = 0x02,
    BMP_FORMAT_RGBA8888 = 0x03,
    BMP_FORMAT_MONO4 = 0x04,
    BMP_FORMAT_ARGB6666 = 0x05,
    BMP_FORMAT_ABGR6666 = 0x06,
    BMP_FORMAT_RGBA5658 = 0x07
} bitmap_format_t;

// Bitmap header structure
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t alpha;
    uint32_t data_size;
    uint8_t compress_type;
} bitmap_header_t;

// Load bitmap from file
uint8_t* bitmap_tool_load_file(const char *filename, uint32_t *width, uint32_t *height, 
                                uint8_t *format, uint8_t *alpha, uint8_t *compress_type);

// Load RGB565 picture
uint8_t* bitmap_tool_load_rgb565_pic(const char *filename, uint32_t *width, uint32_t *height);

// Get pixel buffers for various formats
uint8_t* get_rgba5658_pixel_buff(const char *filename, uint32_t *size, bool is_swap_color);
uint8_t* get_565_pixel_buff(const char *filename, uint32_t *size, bool is_swap_color);
uint8_t* get_565_pixel_buff_douiv6(const char *filename, uint32_t *size, bool is_swap_color);
uint8_t* get_mono4_pixel_buff(const char *filename, uint32_t *size);
uint8_t* get_222_pixel_buff(const char *filename, uint32_t *size);
uint8_t* get_6666_pixel_buff(const char *filename, uint32_t *size, bool swap_color);
uint8_t* get_8888_pixel_buff(const char *filename, uint32_t *size);

#endif // BITMAP_TOOL_H
