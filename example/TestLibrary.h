#ifndef SaveLayerTest_h
#define SaveLayerTest_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <cstdio>

#include "include/core/SkColorSpace.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRRect.h"
#include "include/core/SkFont.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFontMgr.h"


#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

#include "include/effects/SkImageFilters.h"

#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/core/SkStreamPriv.h"

#include "tools/Resources.h" // GetResourceAsData

#define RENDERING 0
std::string skpPath = "skp/saveLayerTest.skp";

class TestLibrary {
public:
    TestLibrary() {}
    static sk_sp<const GrGLInterface> sInterface;
    static sk_sp<GrDirectContext> sContext;


    static void init() {
        SkGraphics::Init();
        sInterface = GrGLMakeNativeInterface();
        if (!sInterface) {
            fprintf(stderr, "GrGLMakeNativeInterface failed\n");
        }
        sContext = GrDirectContexts::MakeGL(sInterface);
        if (!sContext) {
            fprintf(stderr, "GrDirectContext::MakeGL failed\n");
        }
    }

    static sk_sp<SkSurface> CreateSurface(int width, int height, int fboId) {
        GrGLFramebufferInfo info;
        info.fFBOID = fboId; // 기본 FBO (보통 X11, EGL 등에서)
        info.fFormat = GR_GL_RGBA8;
        int sampleCnt = 0; // No multisampling
        int stencil = 8;   // 8 bits stencil buffer

        
        // Skia용 BackendRenderTarget 생성
        auto target = GrBackendRenderTargets::MakeGL(width, height, sampleCnt, stencil, info);

        // Create SkSurface
        sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();        
        sk_sp<SkSurface> surface(SkSurfaces::WrapBackendRenderTarget(sContext.get(),
                                                                    target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    kRGBA_8888_SkColorType,
                                                                    colorSpace,
                                                                    nullptr));
        return surface;
    }

    static void flush() {
        sContext->flush();
        sContext->submit();    
    }

    static void reset() {
        sContext.reset();
        sInterface.reset();
    }
    
    void SaveSkp(sk_sp<SkPicture> picture, const char* filename);
    void Draw(SkCanvas *canvas);
    void ImageRenderDrawTest(SkCanvas *canvas);
    void RenderImageToGpuCanvas(SkCanvas* canvas, const char* imagePath);
    void DrawSquare(SkCanvas* canvas);
    void TestFunc(int width, int height, sk_sp<SkSurface> surface);


};
#endif