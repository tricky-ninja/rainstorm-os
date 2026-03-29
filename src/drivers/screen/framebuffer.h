#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct framebuffer_t {
    uint32_t *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    bool is_ready;
} framebuffer_t;

typedef struct fb_color_t
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} fb_color_t;


void fb_init(framebuffer_t fb);
void fb_clear_screen(fb_color_t clear_color);
void fb_plot_pixel(uint32_t x, uint32_t y, fb_color_t color);
fb_color_t fb_get_pixel(uint32_t x, uint32_t y);
void fb_draw_image(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t *pixels);