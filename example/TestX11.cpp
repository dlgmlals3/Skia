#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <cstdio>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRRect.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"

int main() {
    SkGraphics::Init();

    // X11 + GLX 윈도우 생성
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        fprintf(stderr, "XOpenDisplay failed\n");
        return -1;
    }

    int screen = DefaultScreen(display);
    int visualAttribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* vi = glXChooseVisual(display, screen, visualAttribs);
    if (!vi) {
        fprintf(stderr, "glXChooseVisual failed\n");
        return -1;
    }
    Colormap cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    Window win = XCreateWindow(display, RootWindow(display, vi->screen),
                               0, 0, 800, 600, 0,
                               vi->depth, InputOutput, vi->visual,
                               CWColormap | CWEventMask, &swa);

    XStoreName(display, win, "Skia X11 OpenGL (Ganesh)");
    XMapWindow(display, win);
    GLXContext glctx = glXCreateContext(display, vi, nullptr, GL_TRUE);
    glXMakeCurrent(display, win, glctx);

    // Skia용 BackendRenderTarget 생성
    int width = 800;
    int height = 600;

    GrGLFramebufferInfo info;
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();        

    info.fFBOID = 0; // 기본 FBO (보통 X11, EGL 등에서)
    info.fFormat = GR_GL_RGBA8;
    int sampleCnt = 0; // No multisampling
    int stencil = 8;   // 8 bits stencil buffer

    // Create GrBackendRenderTarget for Skia
    auto target = GrBackendRenderTargets::MakeGL(width, height, sampleCnt, stencil, info);
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface) {
        fprintf(stderr, "GrGLMakeNativeInterface failed\n");
        return -1;
    }
    sk_sp<GrDirectContext> fContext = GrDirectContexts::MakeGL(interface);
    if (!fContext) {
        fprintf(stderr, "GrDirectContext::MakeGL failed\n");
        return -1;
    }
    //auto fContext = GrDirectContexts::MakeGL(fBackendContext, fDisplayParams->grContextOptions());

    // Create SkSurface
    sk_sp<SkSurface> surface(SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                                target,
                                                                kBottomLeft_GrSurfaceOrigin,
                                                                kRGBA_8888_SkColorType,
                                                                colorSpace,
                                                                nullptr));

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorRED);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    fContext->flush();
    fContext->submit();    
    glXSwapBuffers(display, win);

    // X11 이벤트 루프
    bool running = true;
    while (running) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            running = false;
        }
    }

    // 정리
    fContext.reset();
    glXMakeCurrent(display, None, glctx);
    glXDestroyContext(display, glctx);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    SkGraphics::PurgeAllCaches();

    /*
    // Skia GL 인터페이스 & 컨텍스트 생성
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface) {
        fprintf(stderr, "GrGLMakeNativeInterface failed\n");
        return -1;
    }

    sk_sp<GrDirectContext> grCtx = GrDirectContexts::MakeGL(interface);
    if (!grCtx) {
        fprintf(stderr, "GrDirectContexts::MakeGL failed\n");
        return -1;
    }

    // Skia용 BackendRenderTarget 생성
    int width = 800;
    int height = 600;

    GrGLFramebufferInfo info;
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();        

    info.fFBOID = 0; // 기본 FBO (보통 X11, EGL 등에서)
    info.fFormat = GR_GL_RGBA8;
    int sampleCnt = 0; // No multisampling
    int stencil = 8;   // 8 bits stencil buffer

    // Create GrBackendRenderTarget for Skia
    auto target = GrBackendRenderTargets::MakeGL(width, height, sampleCnt, stencil, info);


    // Create SkSurface
    sk_sp<SkSurface> surface(SkSurfaces::WrapBackendRenderTarget(grCtx.get(),
                                                                target,
                                                                kBottomLeft_GrSurfaceOrigin,
                                                                kRGBA_8888_SkColorType,
                                                                colorSpace,
                                                                nullptr));

    // Skia draw
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);
    canvas->drawCircle(400, 300, 100, paint);

    //surface->flush();
    grCtx->submit();
    glXSwapBuffers(display, win);

    // X11 이벤트 루프
    bool running = true;
    while (running) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            running = false;
        }
    }

    // 정리
    grCtx.reset();
    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glctx);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    SkGraphics::PurgeAllCaches();
    */
    return 0;
}
