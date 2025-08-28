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
#include "include/core/SkBitmap.h"

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

#define RENDERING 1
#define DRAW_CONTEXT 2

int testType = DRAW_CONTEXT;

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
    void DrawLineText(SkCanvas *canvas);
    // Bitmap_drawsNothing.cpp

    SkBitmap getBitmap(SkCanvas *canvas) {
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::MakeN32(2, 2, kPremul_SkAlphaType));
        bitmap.erase(0x7fff7f3f, SkIRect(SkIRect::MakeWH(1,1)));
        bitmap.erase(0x7fff7f3f, SkIRect::MakeWH(1, 1));
        bitmap.erase(0x7f7f3fff, SkIRect::MakeXYWH(0, 1, 1, 1));
        bitmap.erase(0x7f3fff7f, SkIRect::MakeXYWH(1, 0, 1, 1));
        bitmap.erase(0x7f1fbf5f, SkIRect::MakeXYWH(1, 1, 1, 1));
        canvas->scale(50, 50);
        //canvas->drawImage(bitmap.asImage(), 0, 0);
        return bitmap;
    }

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_dimensions.cpp
    void BitmapTest9(SkCanvas *canvas) {
    }

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_copy_operator.cpp  
    void BitmapTest8(SkCanvas *canvas) {
    }

    void BitmapTest7(SkCanvas *canvas) {
        // 감마는 픽셀 값과 실제 빛(밝기) 사이의 비선형 관계를 정의하는 지수값
        // 감마(gamma) = 밝기를 표현하는 곡선(함수) 특성
        SkBitmap bitmap;
        bitmap.setInfo(SkImageInfo::MakeN32(2, 2, kPremul_SkAlphaType, 
            SkColorSpace::MakeSRGB()));
        SkColorSpace *cs = bitmap.colorSpace();
        SkDebugf("gammaClosetoSRGB : %s  gammaIsLinear : %s isSRGB : %s\n", 
            cs->gammaCloseToSRGB() ? "true" : "false",
            cs->gammaIsLinear() ? "true" : "false",
            cs->isSRGB() ? "true" : "false"
        );
    }

    // Bitmap_bounds.cpp
    void BitmapTest6(SkCanvas *canvas) {
        // https://github.com/google/skia/blob/main/docs/examples/Bitmap_bounds.cpp
        SkBitmap bitmap = getBitmap(canvas);
        //canvas->scale(.5f, .5f);
        SkIRect bounds = bitmap.bounds();
        SkDebugf("bounds = [%d %d %d %d]\n", 
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
        
        for (int i=0; i<bounds.width(); i++) {
            for (int j=0; j<bounds.height(); j++) {
                canvas->drawImage(bitmap.asImage(), i, j);
            }
        }
    }


    void BitmapTest5(SkCanvas *canvas) {
        // Erase Test
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::MakeN32(2, 2, kPremul_SkAlphaType));
        bitmap.erase(0x7fff7f3f, SkIRect(SkIRect::MakeWH(1,1)));
        bitmap.erase(0x7fff7f3f, SkIRect::MakeWH(1, 1));
        bitmap.erase(0x7f7f3fff, SkIRect::MakeXYWH(0, 1, 1, 1));
        bitmap.erase(0x7f3fff7f, SkIRect::MakeXYWH(1, 0, 1, 1));
        bitmap.erase(0x7f1fbf5f, SkIRect::MakeXYWH(1, 1, 1, 1));
        canvas->scale(50, 50);
        canvas->drawImage(bitmap.asImage(), 0, 0);
        canvas->drawImage(bitmap.asImage(), .5f, .5f);
    }

    void BitmapTest4(SkCanvas *canvas) {
        // 픽셀당 바이트 수 계산
        const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
        SkImageInfo info = SkImageInfo::MakeA8(1, 1);
        SkBitmap bitmap;
        for (SkColorType colorType : {
            kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
            kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
            kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
            kGray_8_SkColorType, kRGBA_F16_SkColorType} ) {
            bitmap.setInfo(info.makeColorType(colorType));
            SkDebugf("color: k" "%s" "_SkColorType" "%*s" "bytesPerPixel: %d\n",
                    colors[colorType], (int)(13 - strlen(colors[colorType])), " ",
                    bitmap.bytesPerPixel());
        }
        bitmap.getPixels();

        // color: kUnknown_SkColorType      bytesPerPixel: 0
        // color: kAlpha_8_SkColorType      bytesPerPixel: 1
        // color: kRGB_565_SkColorType      bytesPerPixel: 2
        // color: kARGB_4444_SkColorType    bytesPerPixel: 2
        // color: kRGBA_8888_SkColorType    bytesPerPixel: 4
        // color: kRGB_888x_SkColorType     bytesPerPixel: 4
        // color: kBGRA_8888_SkColorType    bytesPerPixel: 4
        // color: kRGBA_1010102_SkColorType bytesPerPixel: 4
        // color: kGray_8_SkColorType       bytesPerPixel: 4
        // color: k_SkColorType             bytesPerPixel: 1
        // color: k_SkColorType             bytesPerPixel: 8
    }

    void BitmapTest3(SkCanvas *canvas) {
        const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
        SkPixmap pixmap(SkImageInfo::MakeA8(16, 32), nullptr, 64);
        // 전체 이미지 16(width) * 32(height) = 512 픽셀, 1채널이라 512 바이트
        // 한줄 저장하려면 16 바이트 필요, rowBytes 64 바이트 지정했으니 한줄 건너뛰기 가능
        // 전체 메모리 요구량 = 32 * 64 = 2048 바이트
        // 픽셀 데이터는 512 바이트면 충분하지만, rowBytes가 64 바이트라서 패딩이 생겨서 2K 필요.

        SkDebugf("alpha %lu type : k" "%s" "_SkAphaType\n", 
            pixmap.computeByteSize(), 
            alphas[pixmap.alphaType()]);
    }

    void BitmapTest2(SkCanvas *canvas) {
        https://github.com/google/skia/blob/main/docs/examples/Bitmap_ComputeIsOpaque.cpp
        SkBitmap bitmap;
        bitmap.setInfo(SkImageInfo::Make(2, 2, kN32_SkColorType, kPremul_SkAlphaType));
        for (int index = 0; index < 2; ++index) {
            bitmap.allocPixels();
            bitmap.eraseColor(0x00000000);
            SkDebugf("computeIsOpaque : %s\n", SkBitmap::ComputeIsOpaque(bitmap) ? "true" : "false");
            bitmap.eraseColor(0xFFFFFFFF);
            SkDebugf("computeIsOpaque : %s\n", SkBitmap::ComputeIsOpaque(bitmap) ? "true" : "false");
            //bitmap.setInfo(bitmap.info().makeAlphaType(kOpaque_SkAlphaType));
        }
    }

    // https://github.com/google/skia/blob/main/docs/examples/Anti_Alias.cpp
    void BitmapTest(SkCanvas *canvas) {
        const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
        SkPixmap pixmap(SkImageInfo::MakeA8(16, 32), nullptr, 64);
        SkDebugf("alpha %lu type : k" "%s" "_SkAphaType\n", 
            pixmap.computeByteSize(), 
            alphas[pixmap.alphaType()]);
    }
};
#endif