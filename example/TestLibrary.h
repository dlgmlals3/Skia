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
#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkRect.h"
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

    sk_sp<SkImage> getImage() {    
        // 첫 번째 이미지 로드 및 그리기 (배경)
        auto bgData = GetResourceAsData("images/ducky.jpg");
        sk_sp<SkImage> bgImage = SkImages::DeferredFromEncodedData(bgData, kPremul_SkAlphaType);
        return bgImage;
    }

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

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_height.cpp



    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_getPixels.cpp    
    void Bitmap_getPixel(SkCanvas* canvas) {
        SkBitmap bitmap;
        // 픽셀 내용(또는 픽셀 버퍼)이 바뀔 때마다 달라지는 식별자예요
        SkDebugf("alloc id %u\n", bitmap.getGenerationID());
        bitmap.allocPixels(SkImageInfo::MakeN32(64, 64, kPremul_SkAlphaType));
        SkDebugf("alloc id %u\n", bitmap.getGenerationID());        
        bitmap.eraseColor(SK_ColorRED);
        SkDebugf("erase id %u\n", bitmap.getGenerationID());


        SkBitmap bitmap2;
        bitmap2.setInfo(SkImageInfo::MakeN32(4, 4, kPremul_SkAlphaType));
        bitmap2.allocPixels();
        bitmap2.eraseColor(0x00000000);
        void *baseAddr = bitmap2.getPixels();
        *(SkPMColor*)baseAddr = 0xFFFFFFFF;

        // getColor를 사용해서 특정 칼러값을 가져올수 있음.
        SkDebugf("bitmap.getColor(0, 1) %c= 0x00000000\n",
            bitmap2.getColor(0, 1)  == 0x00000000 ? '=' : '!');
        SkDebugf("bitmap.getColor(0, 0) %c= 0xFFFFFFFF\n",
            bitmap2.getColor(0, 0)  == 0xFFFFFFFF ? '=' : '!');
    }

    void Bitmap_getColor(SkCanvas* canvas) {
        const int w = 4;
        const int h = 4;
        SkColor colors[][w] = {
            { 0x00000000, 0x2a0e002a, 0x55380055, 0x7f7f007f },
            { 0x2a000e2a, 0x551c1c55, 0x7f542a7f, 0xaaaa38aa },
            { 0x55003855, 0x7f2a547f, 0xaa7171aa, 0xd4d48dd4 },
            { 0x7f007f7f, 0xaa38aaaa, 0xd48dd4d4, 0xffffffff }
        };

        for (int y = 0; y < h; ++y) {
            SkDebugf("(0, %d) ", y);
            for (int x = 0; x < w; ++x) {
                SkDebugf("0x%08x%c", colors[y][x], x == w - 1 ? '\n' : ' ');
            }
        }
        // row bytes는 4 bytes(32bit) * 4개 픽셀 
        // pixmap은 뭐지?
        SkPixmap pixmap(SkImageInfo::MakeN32Premul(w, h), colors, w * 4);
        SkBitmap source;
        source.installPixels(pixmap);

        for (int y = 0; y < h; ++y) {
            SkDebugf("(0, %d) ", y);
            for (int x = 0; x < w; ++x) {
                SkDebugf("0x%08x%c", source.getColor(x, y), x == w - 1 ? '\n' : ' ');
            }
        }

        canvas->scale(100, 100);
        canvas->drawImage(source.asImage(), 0, 0);
    }


    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_getBounds.cpp
    void Bitmap_getBounds(SkCanvas *canvas) {
        SkIRect bounds, bounds2;
        SkBitmap source, source1, source2;
        sk_sp<SkImage> image = getImage();
        source.allocPixels(SkImageInfo::MakeN32(image->width(), image->height(), kPremul_SkAlphaType));
        image->readPixels(source.pixmap(), 0, 0);
        // 비트맵 bound를 가져와서 offset 먹히는게 가능.
        source.getBounds(&bounds);
        SkDebugf("bounds : %d %d %d %d\n", bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
                
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->scale(.25f, .25f);
        bounds.offset(100, 100);
        source.extractSubset(&source1, bounds);
        
        source.getBounds(&bounds2);
        bounds2.inset(100, 100);
            
        /*
            Insets SkIRect by (dx,dy).
            If dx is positive, makes SkIRect narrower. 
            If dx is negative, makes SkIRect wider. If dy is positive, makes SkIRect shorter. 
            If dy is negative, makes SkIRect taller.
            Parameters:
            dx – offset added to fLeft and subtracted from fRight
            dy – offset added to fTop and subtracted from fBottom
        */
        source.extractSubset(&source2, bounds2);
        SkIRect r = source2.getSubset();
        // inset의 경우 양쪽에서 안쪽으로 축소됨. (lt 늘어나고, Rb 줄어듬)  
        SkDebugf("bounds2: %d %d %d %d\n", r.fLeft, r.fTop, r.fRight, r.fBottom);

        canvas->drawImage(source.asImage(), 0, 0);
        canvas->drawImage(source1.asImage(), 500, 0);        
        canvas->drawImage(source2.asImage(), 1000, 0);
    } 

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_extractSubset.cpp
    void Bitmap_extractSubset(SkCanvas *canvas) {        
        sk_sp<SkImage> image = getImage();
        SkBitmap source;
        source.allocPixels(SkImageInfo::MakeN32(image->width(), image->height(), kPremul_SkAlphaType));
        image->readPixels(source.pixmap(), 0, 0);

        SkIRect bounds, s;        
        source.getBounds(&bounds);
        SkDebugf("bounds : %d %d %d %d\n", bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
        
        /*
            bool SkBitmap::extractSubset(SkBitmap *dst, const SkIRect &subset) const
            Shares SkPixelRef with dst. Pixels are not copied; 
            SkBitmap and dst point to the same pixels; 
            dst bounds() are set to the intersection of subset and the original bounds().
            subset may be larger than bounds(). Any area outside of bounds() is ignored.
            Any contents of dst are discarded.
            Return false if: - dst is nullptr - SkPixelRef is nullptr - subset does not intersect bounds()
            Parameters:
            dst – SkBitmap set to subset
            subset – rectangle of pixels to reference
        */
        canvas->drawImage(source.asImage(), 0, 0);
        SkBitmap subset;
        for (int left : {-100, 0, 100, 1000}) {
            for (int right : {0, 100, 1000}) {
                SkIRect b = SkIRect::MakeLTRB(left, 0, right, 200);
                bool success = source.extractSubset(&subset, b);
                // 겹치는 부분이 없으면 false 리턴하고 subset는 비어있음. !!
                SkDebugf("subset : %s : %4d %4d %4d %4d\n", 
                    success ? "true" : "false",
                    b.fLeft, b.fTop, b.fRight, b.fBottom);
                canvas->drawImage(subset.asImage(), 0, 0);
            }
        }

    }



    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_extractAlpha_3.cpp
    void Bitmap_extractAlpha3(SkCanvas *canvas) {
        SkBitmap alpha, bitmap;
        bitmap.allocN32Pixels(200, 200);
        SkCanvas offscreen(bitmap);
        offscreen.clear(0);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(10);
        paint.setStyle(SkPaint::kFill_Style); // setStrokeWidth 무시됨..
        //paint.setStyle(SkPaint::kStroke_Style); // 외곽선만 그림 (setStrokeWidth 적용됨)
        //paint.setStyle(SkPaint::kStrokeAndFill_Style); // 외곽선과 내부 모두 그림
        
        offscreen.drawCircle(50, 50, 30, paint);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10, true));

        SkIPoint offset;
        bitmap.extractAlpha(&alpha, &paint, &offset);
        paint.setColor(SK_ColorRED);
        canvas->drawImage(bitmap.asImage(), 0, -offset.fY, SkSamplingOptions(), &paint);
        canvas->drawImage(alpha.asImage(), 100 + offset.fX, 0, SkSamplingOptions(), &paint);
    }

    void BitmapAlphaTest2(SkCanvas *canvas) {
        // 마스크 필터는 경계만 흐려지고
        // 이미지 필터는 이미지 전체가 흐려진다.

        sk_sp<SkMaskFilter> maskFilter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 25, true);
        //SkImageFilters::CropRect cropRect(SkRect::MakeWH(0, 100));
        sk_sp<SkImageFilter> blurFilter = SkImageFilters::Blur(25, 25, nullptr, nullptr);
        sk_sp<SkImage> image = getImage();

        SkBitmap alpha, bitmap;
        bitmap.allocN32Pixels(300, 300);
        SkCanvas offscreen(bitmap);
        offscreen.clear(0);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(20);
        offscreen.drawCircle(50, 50, 39, paint);
        //offscreen.drawImage(image, 0, 0, SkSamplingOptions(), nullptr);    
        paint.setMaskFilter(maskFilter);          
        paint.setImageFilter(blurFilter);
      
        SkIPoint offset;
        bitmap.extractAlpha(&alpha, &paint, &offset);
        paint.setColor(SK_ColorRED);
        canvas->drawImage(bitmap.asImage(), 0, -offset.fY, SkSamplingOptions(), &paint);
        canvas->drawImage(alpha.asImage(), 100 + offset.fX, 0, SkSamplingOptions(), &paint);
    }

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_extractAlpha.cpp
    void BitmapAlphaTest(SkCanvas *canvas) {
        SkBitmap alpha, bitmap;
        bitmap.allocN32Pixels(100, 100);
        // rgba를 모두 0으로 초기화
        SkCanvas offscreen(bitmap);
        offscreen.clear(0);
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(20);
        offscreen.drawCircle(50, 50, 40, paint);
        // 이때 원만 알파가 1로 채워짐.
        
        bitmap.extractAlpha(&alpha);        
        paint.setColor(SK_ColorRED);
        canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
        canvas->drawImage(alpha.asImage(), 100, 0, SkSamplingOptions(), &paint);
    }

    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_dimensions.cpp
    void BitmapTest9(SkCanvas *canvas) {
        SkBitmap bitmap;
        bitmap.setInfo(SkImageInfo::MakeN32(2, 2, kPremul_SkAlphaType));
        SkISize dimension = bitmap.dimensions();
        SkRect rect;
        bitmap.getBounds(&rect);
        SkRect dimensionAsBound = SkRect::Make(dimension);
        SkDebugf("dimensionAsBound %c=bounds\n", dimensionAsBound == rect ? '=' : '!');
        SkDebugf("bounds : %f %f %f %f", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    }


    // https://github.com/google/skia/blob/main/docs/examples/Bitmap_copy_operator.cpp  
    void BitmapTest8(SkCanvas *canvas) {
        SkBitmap bitmap;
        sk_sp<SkColorSpace> cs= SkColorSpace::MakeSRGB();
        SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        SkImageInfo info2 = SkImageInfo::MakeN32(2, 2, kOpaque_SkAlphaType , SkColorSpace::MakeSRGB());
        bitmap.tryAllocPixels(info);
        SkBitmap copy = bitmap;;

        SkDebugf("original has pixels after copy : %s\n", bitmap.getPixels() ? "true" : "false");
        SkDebugf("copy has pixels : %s\n", copy.getPixels() ? "true" : "false");    
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