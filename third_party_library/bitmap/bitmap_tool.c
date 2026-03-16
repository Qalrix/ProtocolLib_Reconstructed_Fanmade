// bitmap_tool.c
#include "bitmap_tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// External image loading library (stb_image)
extern "C" {
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    
    #define STB_IMAGE_RESIZE_IMPLEMENTATION
    #include "stb_image_resize.h"
}

uint8_t* bitmap_tool_load_rgb565_pic(const char *filename, uint32_t *width, uint32_t *height)
{
    if (!filename || !width || !height) {
        return NULL;
    }
    
    printf("err:bitmap load file %s failed\n", filename);
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    printf("filename:%s filesize:%ld\n", filename, filesize);
    
    uint8_t *buffer = (uint8_t *)malloc(filesize);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }
    
    fread(buffer, 1, filesize, fp);
    fclose(fp);
    
    // Parse bitmap header
    uint32_t bfOffBits = *(uint32_t*)(buffer + 10);
    uint32_t biWidth = *(uint32_t*)(buffer + 18);
    uint32_t biHeight = *(uint32_t*)(buffer + 22);
    uint16_t biBitCount = *(uint16_t*)(buffer + 28);
    uint32_t biSizeImage = *(uint32_t*)(buffer + 40);
    
    printf("image:%s, width=%d, height=%d, bih->biHeight = %d, format=%d, rgb_data_offset=%d, file_head_size=%d, info_head_size=%d, sizeof(font_head_t):%d, bih->biSizeImage:%d\n",
           filename, biWidth, biHeight, biHeight, biBitCount, bfOffBits, 14, 40, 0, biSizeImage);
    
    uint8_t *rgb_data = buffer + bfOffBits;
    uint32_t rgb_size = filesize - bfOffBits;
    
    // Convert RGB565 to raw format if needed
    uint8_t *output = (uint8_t *)malloc(rgb_size);
    if (output) {
        memcpy(output, rgb_data, rgb_size);
    }
    
    free(buffer);
    
    printf("load %s complete size:%d\n", filename, rgb_size);
    
    *width = biWidth;
    *height = biHeight;
    
    return output;
}

uint8_t* bitmap_tool_load_file(const char *filename, uint32_t *width, uint32_t *height, 
                                uint8_t *format, uint8_t *alpha, uint8_t *compress_type)
{
    if (!filename || !width || !height) {
        return NULL;
    }
    
    int w, h, channels;
    uint8_t *data = stbi_load(filename, &w, &h, &channels, 4); // Force 4 channels (RGBA)
    
    if (!data) {
        printf("image_load_buff error\n");
        return NULL;
    }
    
    *width = w;
    *height = h;
    *alpha = (channels == 4) ? 1 : 0;
    
    printf("img %s %d x %d format:%d alpha:%d,compress type:%d\n", 
           filename, w, h, channels, *alpha, *compress_type);
    
    return data;
}

uint8_t* get_jpg_pixel_buff(const char *filename, uint32_t *size)
{
    if (!filename || !size) {
        return NULL;
    }
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    printf("img %s %d x %d\n", filename, 0, 0);
    
    if (file_size > 1024 * 1024) { // 1MB max
        printf("err:file %s size(%ld) more than max buf size\n", filename, file_size);
        fclose(fp);
        return NULL;
    }
    
    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }
    
    fread(buffer, 1, file_size, fp);
    fclose(fp);
    
    printf("file %s size:%ld\n", filename, file_size);
    
    *size = file_size;
    return buffer;
}

uint8_t* get_565_pixel_buff(const char *filename, uint32_t *size, bool is_swap_color)
{
    if (!filename || !size) {
        return NULL;
    }
    
    printf("file_name:%s, use func:bitmap_tool_load_rgb565_pic load file..\n", filename);
    
    uint32_t width, height;
    uint8_t *data = bitmap_tool_load_rgb565_pic(filename, &width, &height);
    
    if (!data) {
        return NULL;
    }
    
    // Convert to RGB565 format if needed
    uint32_t pixel_count = width * height;
    uint32_t data_size = pixel_count * 2; // 2 bytes per pixel for RGB565
    
    uint8_t *output = (uint8_t *)malloc(data_size);
    if (output) {
        memcpy(output, data, data_size < *size ? data_size : *size);
    }
    
    free(data);
    
    *size = data_size;
    return output;
}

uint8_t* get_565_pixel_buff_douiv6(const char *filename, uint32_t *size, bool is_swap_color)
{
    if (!filename || !size) {
        return NULL;
    }
    
    int w, h, channels;
    uint8_t *data = stbi_load(filename, &w, &h, &channels, 4);
    
    if (!data) {
        return NULL;
    }
    
    printf("img %s %d x %d alpha = %d,is_swap_color = %d\n", 
           filename, w, h, (channels == 4), is_swap_color);
    
    // Convert to RGB565 format
    uint32_t pixel_count = w * h;
    uint32_t data_size = pixel_count * 2;
    uint8_t *output = (uint8_t *)malloc(data_size);
    
    if (output) {
        uint16_t *dst = (uint16_t *)output;
        uint8_t *src = data;
        
        for (uint32_t i = 0; i < pixel_count; i++) {
            uint8_t r = src[0];
            uint8_t g = src[1];
            uint8_t b = src[2];
            
            // RGB565 conversion
            uint16_t pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            
            if (is_swap_color) {
                pixel = (pixel >> 8) | ((pixel & 0xFF) << 8);
            }
            
            *dst++ = pixel;
            src += 4;
        }
    }
    
    stbi_image_free(data);
    
    *size = data_size;
    return output;
}

uint8_t* get_mono4_pixel_buff(const char *filename, uint32_t *size)
{
    if (!filename || !size) {
        return NULL;
    }
    
    int w, h, channels;
    uint8_t *data = stbi_load(filename, &w, &h, &channels, 1); // Force grayscale
    
    if (!data) {
        return NULL;
    }
    
    printf("img %s w : %d , h : %d\n", filename, w, h);
    
    // Convert to 4-bit monochrome
    uint32_t data_size = (w * h + 1) / 2; // 2 pixels per byte
    uint8_t *output = (uint8_t *)malloc(data_size);
    
    if (output) {
        memset(output, 0, data_size);
        
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint8_t pixel = data[y * w + x];
                uint8_t bit = (pixel > 128) ? 0xF : 0x0; // Threshold
                
                uint32_t idx = (y * w + x) / 2;
                if ((y * w + x) % 2 == 0) {
                    output[idx] = (output[idx] & 0x0F) | (bit << 4);
                } else {
                    output[idx] = (output[idx] & 0xF0) | bit;
                }
            }
        }
    }
    
    stbi_image_free(data);
    
    *size = data_size;
    return output;
}

uint8_t* get_6666_pixel_buff(const char *filename, uint32_t *size, bool swap_color)
{
    if (!filename || !size) {
        return NULL;
    }
    
    printf("err:get argb6666 pixel load file %s error(%d)\n", filename, -1);
    
    int w, h, channels;
    uint8_t *data = stbi_load(filename, &w, &h, &channels, 4);
    
    if (!data) {
        return NULL;
    }
    
    printf("img %s %d x %d format:%d alpha:%d,compress type:%d swap_color:%d\n",
           filename, w, h, channels, (channels == 4) ? 1 : 0, 0, swap_color);
    
    // Convert to ARGB6666 format
    uint32_t pixel_count = w * h;
    uint32_t data_size = pixel_count * 3; // 24 bits per pixel (6 bits per channel)
    uint8_t *output = (uint8_t *)malloc(data_size);
    
    if (output) {
        uint8_t *dst = output;
        uint8_t *src = data;
        
        for (uint32_t i = 0; i < pixel_count; i++) {
            uint8_t r = src[0] >> 2; // 8-bit to 6-bit
            uint8_t g = src[1] >> 2;
            uint8_t b = src[2] >> 2;
            uint8_t a = src[3] >> 2;
            
            if (swap_color) {
                // Store in ABGR6666 order
                dst[0] = (a << 2) | (b >> 4);
                dst[1] = ((b & 0x0F) << 4) | (g >> 2);
                dst[2] = ((g & 0x03) << 6) | r;
            } else {
                // Store in ARGB6666 order
                dst[0] = (a << 2) | (r >> 4);
                dst[1] = ((r & 0x0F) << 4) | (g >> 2);
                dst[2] = ((g & 0x03) << 6) | b;
            }
            
            dst += 3;
            src += 4;
        }
    }
    
    stbi_image_free(data);
    
    printf("compress fastlz,src size:%d,dst size:%d\n", pixel_count * 4, data_size);
    
    *size = data_size;
    return output;
}

uint8_t* get_8888_pixel_buff(const char *filename, uint32_t *size)
{
    if (!filename || !size) {
        return NULL;
    }
    
    int w, h, channels;
    uint8_t *data = stbi_load(filename, &w, &h, &channels, 4);
    
    if (!data) {
        return NULL;
    }
    
    uint32_t data_size = w * h * 4;
    uint8_t *output = (uint8_t *)malloc(data_size);
    
    if (output) {
        memcpy(output, data, data_size);
    }
    
    stbi_image_free(data);
    
    *size = data_size;
    return output;
}
