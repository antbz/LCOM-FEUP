#include "videocard.h"

static uint8_t *video_mem;
static unsigned x_res;
static unsigned y_res;
static unsigned r_mask_size;
static unsigned r_pos;
static unsigned g_mask_size;
static unsigned g_pos;
static unsigned b_mask_size;
static unsigned b_pos;
static unsigned bytes_per_pixel;

int vbe_mode_info(uint16_t mode, vbe_mode_info_t *mode_info) {
  /*struct minix_mem_range mr;
  mr.mr_base = 0;
  mr.mr_limit = 1<<20;
  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM,&mr) != OK) {
    printf("Error setting premissions!");
    return 1;
  }*/

  reg86_t r;

  memset(&r, 0, sizeof(r));

  mmap_t map;
  lm_alloc(sizeof(vbe_mode_info_t),&map);
  
  phys_bytes buf = map.phys;

  r.intno = VIDEOCARD;
  r.ah = VBE_FUNC;
  r.al = GET_MODE_INFO;
  r.cx = LINEAR_FB | mode;
  r.es = PB2BASE(buf);
  r.di = PB2OFF(buf);

  if (sys_int86(&r) != OK) {
    printf("sysint86 FAILED!");
    lm_free(&map);
    return 1;
  }

  if (r.ah != 0) {
    lm_free(&map);
    return 1;
  }
  
  *mode_info = *(vbe_mode_info_t *) map.virt;

  lm_free(&map);

  return 0;
}

int set_vbe_mode(uint16_t mode) {
  struct minix_mem_range mr;
  mr.mr_base = 0;
  mr.mr_limit = 1<<20;
  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM,&mr) != OK) {
    printf("Error setting premissions!");
    return 1;
  }

  reg86_t r;

  memset(&r, 0, sizeof(r));

  r.intno = VIDEOCARD;
  r.ah = VBE_FUNC;
  r.al = SET_VBE_MODE;
  r.bx = LINEAR_FB | mode;

  if (sys_int86(&r) != OK) {
    printf("sysint86 FAILED!");
    return 1;
  }

  return 0;
}

int bitsToBytes(uint8_t bits) {
  double bytes = (double)bits / 8.0;
  return ceil(bytes);
}

void map_video_mem(uint16_t mode) {
  struct minix_mem_range mr;
  vbe_mode_info_t mode_info;
  vbe_mode_info(mode, &mode_info);

  x_res = mode_info.XResolution;
  y_res = mode_info.YResolution;
  bytes_per_pixel = bitsToBytes(mode_info.BitsPerPixel);
  r_mask_size = mode_info.RedMaskSize;
  r_pos = mode_info.RedFieldPosition;
  g_mask_size = mode_info.GreenMaskSize;
  g_pos = mode_info.GreenFieldPosition;
  b_mask_size = mode_info.BlueMaskSize;
  b_pos = mode_info.BlueFieldPosition;
  /*printf("%d\n",x_res);
  printf("%d\n",y_res);
  printf("%d\n",bytes_per_pixel);
  printf("%d\n",r_mask_size);
  printf("%d\n",r_pos);
  printf("%d\n",g_mask_size);
  printf("%d\n",g_pos);
  printf("%d\n",b_mask_size);
  printf("%d\n",b_pos);*/
  unsigned int vram_size = x_res * y_res * bytes_per_pixel;

  mr.mr_base = mode_info.PhysBasePtr;
  mr.mr_limit = mr.mr_base + vram_size;

  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM,&mr) != OK) {
    panic("Error setting premissions");
  }
  
  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED) {
    panic("Error mapping video memory");
  }
}

int set_pixel(uint32_t color, uint16_t x, uint16_t y) {

  
  for (unsigned i = 0; i <  bytes_per_pixel; i++) {
    video_mem[(y * x_res + x) * bytes_per_pixel + i] = color >> (8*i) & 0xFF;
  }

  return 0;
}

int draw_hline(uint32_t color, uint16_t x, uint16_t y, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    set_pixel(color, x, y + i);
  }
  return 0;
}

int draw_rectangle(uint32_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  for (uint16_t i = 0; i < height; i++) {
    draw_hline(color, x + i, y, width);
  }

  return 0;
}

uint32_t build_mask(uint32_t size, uint32_t pos) {
  uint32_t mask = 0x1;
  for (uint32_t i = 0; i < size - 1; i++) {
    mask = (mask << 1) + 1;
  }
  return mask << pos;
}

uint32_t RGB_col(uint16_t col, uint16_t row, uint32_t first, uint8_t step) {
  uint32_t r, g, b, r_mask, g_mask, b_mask;
  
  r_mask = build_mask(r_mask_size, r_pos);
  g_mask = build_mask(g_mask_size, g_pos);
  b_mask = build_mask(b_mask_size, b_pos);
  

  r = ((((first & r_mask) >> r_pos) + col * step) % (1 << r_mask_size)) << r_pos;
  g = ((((first & g_mask) >> g_pos) + row * step) % (1 << g_mask_size)) << g_pos;
  b = (((first & b_mask) + (col + row) * step) % (1 << b_mask_size)) << b_pos;

  return r | g | b;
}

int draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step) {
  uint16_t width = x_res / no_rectangles;
  uint16_t height = y_res / no_rectangles;

  for (uint16_t row = 0; row < no_rectangles; row++) {
    for (uint16_t col = 0; col < no_rectangles; col++) {
      uint32_t color = 0;
      if (bytes_per_pixel == 1) {
        color = (first + (row * no_rectangles + col) * step) % (1 << 8);
      } else {
        color = RGB_col(col, row, first, step);
      }
      // printf("%d\n", color);
      draw_rectangle(color, row*height, col*width, width, height);
    }
  }
  return 0;
}

int draw_xpm(uint16_t x, uint16_t y, xpm_image_t img, uint8_t *map) {
  for (int h = 0; h < img.height; h++) {
    for (int w = 0; w < img.width; w++) {
      set_pixel( map[img.width*h + w], w+x, h+y);
    }
  }
  return 0;
}

int clear_xpm(uint16_t x, uint16_t y, xpm_image_t img) {
  for (int h = y; h < y + img.height; h++) {
    for (int w = x; w < x + img.width; w++) {
      set_pixel(0, h, w);
    }
  }
  return 0;
}

/* Deprecated
int (move_xpm)(xpm_image_t img, uint8_t *map, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {
  for (uint16_t x = xi; x < xf; x+=speed) {
    for (uint16_t y = yi; y < yf; y += speed) {
      draw_xpm(x, y, img, map);
      tickdelay(micros_to_ticks((1/fr_rate) * 1000000));
    }
  }
  return 0;
}*/

