#include <am.h>
#include <x86.h>
#include "ioe-gfx.h"

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long current = inl(RTC_PORT);
  unsigned long elapsed = current - boot_time;
  return elapsed;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  _am_draw_rect_clipped(fb, _screen.width, _screen.height, pixels, x, y, w, h);
}

void _draw_sync() {
}

int _read_key() {
  uint8_t status = inb(0x64);
  if (status & 0x1) {
    int key = inl(0x60);
    return key;
  }
  return _KEY_NONE;
}
