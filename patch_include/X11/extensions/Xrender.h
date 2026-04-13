#ifndef _XRENDER_H_
#define _XRENDER_H_

#include <X11/Xlib.h>

typedef struct {
    int id;
} XRenderPictFormat;

typedef XRenderPictFormat* (*XRenderFindVisualFormatFunc)(Display *dpy, const Visual *visual);

#endif