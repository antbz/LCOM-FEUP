#include <lcom/lcf.h>
#include "defs.h"
#include "math.h"

int set_vbe_mode(uint16_t mode);
int bitsToBytes(uint8_t bits);
void map_video_mem(uint16_t mode);
int set_pixel(uint32_t color, uint16_t x, uint16_t y);
int draw_hline(uint32_t color, uint16_t x, uint16_t y, uint16_t len);
int draw_rectangle(uint32_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
int draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step);
int draw_xpm(uint16_t x, uint16_t y, xpm_image_t img, uint8_t *map);
int clear_xpm(uint16_t x, uint16_t y, xpm_image_t img);
int move_xpm(xpm_image_t img, uint8_t *map, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,  int16_t speed, uint8_t fr_rate);
