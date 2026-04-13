#ifndef _XUTIL_H_
#define _XUTIL_H_

#include <X11/Xlib.h>

typedef struct {
    int depth;
    int bits_per_pixel;
    int scanline_pad;
} XPixmapFormatValues;

#endif