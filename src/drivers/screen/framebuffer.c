#include "framebuffer.h"
#include "utils/memory.h"

#define RED(r)   (((uint32_t)(r) & 0xFF) << 16)
#define GREEN(g) (((uint32_t)(g) & 0xFF) << 8)
#define BLUE(b)  (((uint32_t)(b) & 0xFF))

#define GET_RED(px)   (((px) >> 16) & 0xFF)
#define GET_GREEN(px) (((px) >> 8)  & 0xFF)
#define GET_BLUE(px)  (((px))       & 0xFF)

static framebuffer_t framebuffer = {0};

void fb_init(framebuffer_t fb)
{
    framebuffer.address = fb.address;
    framebuffer.height = fb.height;
    framebuffer.width = fb.width;
    framebuffer.pitch = fb.pitch;
    framebuffer.is_ready = true;
}

void fb_clear_screen(fb_color_t clear_color)
{
    if (!framebuffer.is_ready) return;
    uint32_t color_int = RED(clear_color.red) | GREEN(clear_color.green) | BLUE(clear_color.blue);
    for (size_t y = 0; y < framebuffer.height; y++)
    {
        for (size_t x = 0; x < framebuffer.width; x++)
        {
            framebuffer.address[y * (framebuffer.pitch/4) + x] = color_int;
        }
    }
}

void fb_draw_image(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t *pixels)
{
    if (!framebuffer.is_ready) return;
    if (pixels == NULL) return;
    if (x >= framebuffer.width)  return;
    if (y >= framebuffer.height) return;

    uint32_t draw_width  = (x + width  > framebuffer.width)  ? framebuffer.width  - x : width;
    uint32_t draw_height = (y + height > framebuffer.height) ? framebuffer.height - y : height;

    for (uint32_t row = 0; row < draw_height; row++)
    {
        for (uint32_t col = 0; col < draw_width; col++)
        {
            uint32_t pixel = pixels[row * width + col];
            uint8_t alpha = (pixel >> 24) & 0xFF;

            if (alpha == 0) continue;   // TODO: temporary
            framebuffer.address[(y + row) * (framebuffer.pitch >> 2) + (x + col)] = pixel & 0x00FFFFFF;
        }
    }
}



void fb_plot_pixel(uint32_t x, uint32_t y, fb_color_t color)
{
    if (!framebuffer.is_ready) return;
    if (x >= framebuffer.width)  return;
    if (y >= framebuffer.height) return;

    uint32_t color_int = RED(color.red) | GREEN(color.green) | BLUE(color.blue);
    framebuffer.address[y * (framebuffer.pitch/4) + x] = color_int;
}

fb_color_t fb_get_pixel(uint32_t x, uint32_t y)
{
    if (!framebuffer.is_ready) return (fb_color_t){0,0,0};
    if (x >= framebuffer.width)  return (fb_color_t){0,0,0};
    if (y >= framebuffer.height) return (fb_color_t){0,0,0};

    uint32_t color_int = framebuffer.address[y * (framebuffer.pitch/4) + x];

    return (fb_color_t){
        .red = GET_RED(color_int),
        .green = GET_GREEN(color_int),
        .blue = GET_BLUE(color_int)
    };
}