#ifndef __AM_IOE_GFX_H__
#define __AM_IOE_GFX_H__

#include <stdint.h>

static inline void _am_draw_rect_clipped(uint32_t *fb, int screen_w, int screen_h,
    const uint32_t *pixels, int x, int y, int w, int h) {
  if (w <= 0 || h <= 0) return;

  int x0 = x < 0 ? 0 : x;
  int y0 = y < 0 ? 0 : y;
  int x1 = x + w > screen_w ? screen_w : x + w;
  int y1 = y + h > screen_h ? screen_h : y + h;
  if (x0 >= x1 || y0 >= y1) return;

  for (int j = y0; j < y1; j++) {
    uint32_t *dst = fb + j * screen_w + x0;
    const uint32_t *src = pixels + (j - y) * w + (x0 - x);
    for (int i = 0; i < x1 - x0; i++) {
      dst[i] = src[i];
    }
  }
}

#endif
