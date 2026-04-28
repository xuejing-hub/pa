#include <psg.h>

static byte prev_write;
static int p = 10;

extern int key_state[];

static inline byte key_down(int key) {
    if (key < 0 || key >= 256) return 0;
    return key_state[key] ? 1 : 0;
}

// static inline byte pad1_button(int idx) {
//     switch (idx) {
//         case 1: return key_down(_KEY_G) | key_down(_KEY_J) | key_down(_KEY_SPACE);      // A
//         case 2: return key_down(_KEY_H) | key_down(_KEY_K) | key_down(_KEY_LCTRL);      // B
//         case 3: return key_down(_KEY_T) | key_down(_KEY_TAB);                             // SELECT
//         case 4: return key_down(_KEY_Y) | key_down(_KEY_RETURN);                          // START
//         case 5: return key_down(_KEY_W) | key_down(_KEY_UP);                              // UP
//         case 6: return key_down(_KEY_S) | key_down(_KEY_DOWN);                            // DOWN
//         case 7: return key_down(_KEY_A) | key_down(_KEY_LEFT);                            // LEFT
//         case 8: return key_down(_KEY_D) | key_down(_KEY_RIGHT);                           // RIGHT
//         default: return 0;
//     }
// }

static inline byte pad1_button(int idx) {
    switch (idx) {
        case 1: return key_down(_KEY_G);                                                  // A
        case 2: return key_down(_KEY_H);                                                  // B
        case 3: return key_down(_KEY_T);                                                  // SELECT
        case 4: return key_down(_KEY_Y);                                                  // START
        case 5: return key_down(_KEY_W);                                                  // UP
        case 6: return key_down(_KEY_S);                                                  // DOWN
        case 7: return key_down(_KEY_A);                                                  // LEFT
        case 8: return key_down(_KEY_D);                                                  // RIGHT
        default: return 0;
    }
}

inline byte psg_io_read(word address)
{
    // Joystick 1
    if (address == 0x4016) {
        if (p++ < 9) {
                    return pad1_button(p);
        }
    }
    return 0;
}

inline void psg_io_write(word address, byte data)
{
    if (address == 0x4016) {
        if ((data & 1) == 0 && prev_write == 1) {
            // strobe
            p = 0;
        }
    }
    prev_write = data & 1;
}
