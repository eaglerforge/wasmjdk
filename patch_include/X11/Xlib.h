#ifndef _XLIB_H_
#define _XLIB_H_

#include <stdint.h>

typedef void* Display;
typedef uintptr_t Window;
typedef uintptr_t Pixmap;
typedef uintptr_t Colormap;
typedef uintptr_t Cursor;
typedef uintptr_t GContext;
typedef void* GC;
typedef uintptr_t VisualID;

#define True  1
#define False 0
#define _Xconst const

typedef struct {
    int extents;
} Visual;

typedef struct {
    Visual *visual;
    VisualID visualid;
    int screen;
    int depth;
    int klass;
    unsigned long red_mask, green_mask, blue_mask;
    int colormap_size;
    int bits_per_rgb;
} XVisualInfo;

typedef struct _XImage {
    int width, height;
    int xoffset;
    int format;
    char *data;
    int byte_order;
    int bitmap_unit;
    int bitmap_bit_order;
    int bitmap_pad;
    int depth;
    int bytes_per_line;
    int bits_per_pixel;
} XImage;

char **XGetFontPath(Display*, int*);
int XFreeFontPath(char**);
typedef int (*XErrorHandler)(Display *, void *);

#endif