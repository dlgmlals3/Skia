#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/SkSurfaceTest.h"

int main() {
    SkGraphics::Init();

    // X11 윈도우 생성
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        fprintf(stderr, "XOpenDisplay failed\n");
        return -1;
    }

    int screen = DefaultScreen(display);
    Window win = XCreateSimpleWindow(display, RootWindow(display, screen),
                                      0, 0, 800, 600, 1,
                                      BlackPixel(display, screen),
                                      WhitePixel(display, screen));
    XStoreName(display, win, "Skia X11 Example");
    XSelectInput(display, win, ExposureMask | KeyPressMask);
    XMapWindow(display, win);

    // Skia surface
    SkImageInfo info = SkImageInfo::MakeN32Premul(800, 600);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    SkCanvas* canvas = surface->getCanvas();
    // dlgmlals3
    SkSurfaceTestDraw(canvas);

    // XImage 생성
    SkPixmap pixmap;
    if (!surface->peekPixels(&pixmap)) {
        fprintf(stderr, "Failed to peek pixels\n");
        return -1;
    }

    XImage* ximage = XCreateImage(display, DefaultVisual(display, screen),
                                  24, ZPixmap, 0,
                                  (char*)malloc(pixmap.computeByteSize()),
                                  pixmap.width(), pixmap.height(),
                                  32, pixmap.rowBytes());

    memcpy(ximage->data, pixmap.addr(), pixmap.computeByteSize());

    // 이벤트 루프
    bool running = true;
    while (running) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == Expose) {
            GC gc = XCreateGC(display, win, 0, nullptr);
            XPutImage(display, win, gc, ximage, 0, 0, 0, 0,
                      pixmap.width(), pixmap.height());
            XFreeGC(display, gc);
        } else if (event.type == KeyPress) {
            running = false;
        }
    }

    XDestroyImage(ximage);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    SkGraphics::PurgeAllCaches();
    return 0;
}