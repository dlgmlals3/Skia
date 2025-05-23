
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/window/unix/GaneshGLWindowContext_unix.h"

#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "tools/window/GLWindowContext.h"
#include "tools/window/unix/XlibWindowInfo.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

using skwindow::DisplayParams;
using skwindow::XlibWindowInfo;
using skwindow::internal::GLWindowContext;

namespace {

static bool gCtxErrorOccurred = false;
static int ctxErrorHandler(Display* dpy, XErrorEvent* ev) {
    gCtxErrorOccurred = true;
    return 0;
}

class GLWindowContext_xlib : public GLWindowContext {
public:
    GLWindowContext_xlib(const XlibWindowInfo&, std::unique_ptr<const DisplayParams>);
    ~GLWindowContext_xlib() override;

    void onDestroyContext() override;

protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onSwapBuffers() override;

private:
    GLWindowContext_xlib(void*, std::unique_ptr<const DisplayParams>);

    Display* fDisplay;
    XWindow fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    GLXContext fGLContext;
};

GLWindowContext_xlib::GLWindowContext_xlib(const XlibWindowInfo& winInfo,
                                           std::unique_ptr<const DisplayParams> params)
        : GLWindowContext(std::move(params))
        , fDisplay(winInfo.fDisplay)
        , fWindow(winInfo.fWindow)
        , fFBConfig(winInfo.fFBConfig)
        , fVisualInfo(winInfo.fVisualInfo)
        , fGLContext() {
    fWidth = winInfo.fWidth;
    fHeight = winInfo.fHeight;
    this->initializeContext();
}

using CreateContextAttribsFn = GLXContext(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static sk_sp<const GrGLInterface> make_interface() { return GrGLInterfaces::MakeGLX(); }

sk_sp<const GrGLInterface> GLWindowContext_xlib::onInitializeContext() {
    SkASSERT(fDisplay);
    SkASSERT(!fGLContext);
    sk_sp<const GrGLInterface> interface;
    bool current = false;

    // We attempt to use glXCreateContextAttribsARB as RenderDoc requires that the context be
    // created with this rather than glXCreateContext.
    CreateContextAttribsFn* createContextAttribs = (CreateContextAttribsFn*)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");
    if (createContextAttribs && fFBConfig) {
        // Install Xlib error handler that will set gCtxErrorOccurred
        int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

        // Specifying 3.2 allows an arbitrarily high context version (so long as no 3.2 features
        // have been removed).
        for (int minor = 2; minor >= 0 && !fGLContext; --minor) {
            // Ganesh prefers a core profile which incidentally allows RenderDoc to work correctly.
            for (int profile :
                 {GLX_CONTEXT_CORE_PROFILE_BIT_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB}) {
                gCtxErrorOccurred = false;
                int attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB,
                                 3,
                                 GLX_CONTEXT_MINOR_VERSION_ARB,
                                 minor,
                                 GLX_CONTEXT_PROFILE_MASK_ARB,
                                 profile,
                                 0};
                fGLContext = createContextAttribs(fDisplay, *fFBConfig, nullptr, True, attribs);

                // Sync to ensure any errors generated are processed.
                XSync(fDisplay, False);
                if (gCtxErrorOccurred) {
                    continue;
                }

                if (fGLContext && profile == GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB &&
                    glXMakeCurrent(fDisplay, fWindow, fGLContext)) {
                    current = true;
                    // Look to see if RenderDoc is attached. If so, re-create the context with a
                    // core profile.
                    interface = make_interface();
                    if (interface && interface->fExtensions.has("GL_EXT_debug_tool")) {
                        interface.reset();
                        glXMakeCurrent(fDisplay, None, nullptr);
                        glXDestroyContext(fDisplay, fGLContext);
                        current = false;
                        fGLContext = nullptr;
                    }
                }
                if (fGLContext) {
                    break;
                }
            }
        }
        // Restore the original error handler
        XSetErrorHandler(oldHandler);
    }
    if (!fGLContext) {
        fGLContext = glXCreateContext(fDisplay, fVisualInfo, nullptr, GL_TRUE);
    }
    if (!fGLContext) {
        return nullptr;
    }

    if (!current && !glXMakeCurrent(fDisplay, fWindow, fGLContext)) {
        return nullptr;
    }

    const char* glxExtensions = glXQueryExtensionsString(fDisplay, DefaultScreen(fDisplay));
    if (glxExtensions) {
        if (strstr(glxExtensions, "GLX_EXT_swap_control")) {
            PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT =
                    (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB(
                            (const GLubyte*)"glXSwapIntervalEXT");
            glXSwapIntervalEXT(fDisplay, fWindow, fDisplayParams->disableVsync() ? 0 : 1);
        }
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glXGetConfig(fDisplay, fVisualInfo, GLX_STENCIL_SIZE, &fStencilBits);
    glXGetConfig(fDisplay, fVisualInfo, GLX_SAMPLES_ARB, &fSampleCount);
    fSampleCount = std::max(fSampleCount, 1);

    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    XGetGeometry(fDisplay,
                 fWindow,
                 &root,
                 &x,
                 &y,
                 (unsigned int*)&fWidth,
                 (unsigned int*)&fHeight,
                 &border_width,
                 &depth);
    glViewport(0, 0, fWidth, fHeight);

    return interface ? interface : make_interface();
}

GLWindowContext_xlib::~GLWindowContext_xlib() { this->destroyContext(); }

void GLWindowContext_xlib::onDestroyContext() {
    if (!fDisplay || !fGLContext) {
        return;
    }
    glXMakeCurrent(fDisplay, None, nullptr);
    glXDestroyContext(fDisplay, fGLContext);
    fGLContext = nullptr;
}

void GLWindowContext_xlib::onSwapBuffers() {
    if (fDisplay && fGLContext) {
        glXSwapBuffers(fDisplay, fWindow);
    }
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGaneshGLForXlib(const XlibWindowInfo& winInfo,
                                                   std::unique_ptr<const DisplayParams> params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_xlib(winInfo, std::move(params)));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
