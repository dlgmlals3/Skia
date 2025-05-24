#include "../include/SkSurfaceTest.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/gpu/ganesh/GrTypes.h"
/*
void SkSurfaceTestDraw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    canvas->drawLine(20, 100, 100, 100, p);
}
void SkSurfaceTestDraw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorBLUE);
    lil->draw(big->getCanvas(), 16, 16);
    
    canvas->drawImage(big->makeImageSnapshot(), 0, 0);
}

void SkSurfaceTestDraw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));

    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorBLUE);
    lil->draw(big->getCanvas(), 16, 16);
    
    // 비트맵 수정가능한 영역
    SkPixmap pixmap;
    if (big->peekPixels(&pixmap)) {
        SkBitmap bigBits;
        bigBits.installPixels(pixmap);
        canvas->drawImage(bigBits.asImage(), 0, 0);
    }
}

// null 캔버스는 디버그 용으로 사용함. 실제로 안그려짐..
void SkSurfaceTestDraw(SkCanvas* canvas) {
    const int width = 37;
    const int height = 1000;
    auto surf = SkSurfaces::Null(width, height);
    auto nullCanvas = surf->getCanvas();
    SkDebugf("surface height=%d  canvas height=%d\n", surf->height(),
             nullCanvas->getBaseLayerSize().fHeight);
    SkDebugf("surface width=%d cnavas width=%d\n", surf->width(), 
        nullCanvas->getBaseLayerSize().width());
}


void SkSurfaceTestDraw(SkCanvas* canvas) {
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
    SkPaint paint;

    GrRecordingContext* context = canvas->recordingContext();
    if (!context) {
        canvas->drawString("GPU only!", 20, 40, font, paint);
        return;
    }
    GrDirectContext* direct = context->asDirectContext();
    if (!direct) {
        canvas->drawString("Direct Context only!", 20, 40, font, paint);
        return;
    }

    sk_sp<SkSurface> gpuSurface = SkSurfaces::WrapBackendTexture(direct,
                                            backEndTexture,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kRGBA_8888_SkColorType,
                                            nullptr,
                                            nullptr,
                                            nullptr);
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->drawString("GPU rocks!", 20, 40, font, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
//
// lil->draw(big->getCanvas(), 16, 16)
// big 캔버스에 lil 그리므로 빨강 바탕에 초록 패치 x,y(16, 16)를 그림.

void SkSurfaceTestDraw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> early(big->makeImageSnapshot());
    lil->draw(big->getCanvas(), 16, 16);
    sk_sp<SkImage> later(big->makeImageSnapshot({0, 0, 64, 64}));
    //canvas->drawImage(early, 0, 0);
    canvas->drawImage(later, 0, 0);
}

void SkSurfaceTestDraw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> early(big->makeImageSnapshot());
    lil->draw(big->getCanvas(), 16, 16);
    sk_sp<SkImage> later(big->makeImageSnapshot());
    canvas->drawImage(early, 0, 0);
    canvas->drawImage(later, 128, 0);
}

// Surface_MakeRaster.cpp
// Raster pixel memory Size = info.height * rowBytes
// 코드는 Skia의 SkSurface 메모리를 직접 접근해서 픽셀 값이 변했는지 확인하는 디버깅 예제
void SkSurfaceTestDraw(SkCanvas* canvasReal) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    const size_t rowBytes = 64;
    // memorySize = 3 * 64 = 192
    // 한 줄당 4byte 16개 픽셀이 들어가 있고, 15개 픽셀이 패딩 픽셀
    sk_sp<SkSurface> surface(SkSurfaces::Raster(info, rowBytes, nullptr));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPixmap pixmap;

    if (surface->peekPixels(&pixmap)) {
        // 서피스가 가리키고 있는 픽셀 메모리
        const uint32_t* colorPtr = pixmap.addr32();
        // 서피스가 가지고 있는 픽셀 메모리며, clearWhite 이므로 
        SkPMColor pmWhite = colorPtr[0];
        uint8_t a = (pmWhite >> 24) & 0xFF;
        uint8_t r = (pmWhite >> 16) & 0xFF;
        uint8_t g = (pmWhite >> 8) & 0xFF;
        uint8_t b = (pmWhite >> 0) & 0xFF;
        SkDebugf("WHITE argb : %d %d %d %d\n", a,r,g,b);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        canvas->drawPoint(0, 0, paint);
        canvas->drawPoint(1, 1, paint);
        canvas->drawPoint(2, 2, paint);

        a = (colorPtr[0] >> 24) & 0xFF;
        r = (colorPtr[0] >> 16) & 0xFF;
        g = (colorPtr[0] >> 8) & 0xFF;
        b = (colorPtr[0] >> 0) & 0xFF;
        SkDebugf("BLUE [0][0] argb : %d %d %d %d\n", a,r,g,b);
        a = (colorPtr[17] >> 24) & 0xFF;
        r = (colorPtr[17] >> 16) & 0xFF;
        g = (colorPtr[17] >> 8) & 0xFF;
        b = (colorPtr[17] >> 0) & 0xFF;
        SkDebugf("BLUE [1][1] argb : %d %d %d %d\n", a,r,g,b);
        a = (colorPtr[34] >> 24) & 0xFF;
        r = (colorPtr[34] >> 16) & 0xFF;
        g = (colorPtr[34] >> 8) & 0xFF;
        b = (colorPtr[34] >> 0) & 0xFF;
        SkDebugf("BLUE [2][2] argb : %d %d %d %d\n", a,r,g,b);

        // (1, 1) (2, 2) 픽셀을 제외한 픽셀들은 전부 White 색상
        // 두 픽셀은 Blue
        for (int y = 0; y < info.height(); ++y) {
            for (int x = 0; x < info.width(); ++x) {
                SkDebugf("%c", colorPtr[x] == pmWhite ? '-' : 'x');
            }
            colorPtr += rowBytes / sizeof(colorPtr[0]);
            SkDebugf("\n");
        }
    }
}
*/

// WrapPixels가 
void SkSurfaceTestDraw(SkCanvas*) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    const size_t size = info.computeMinByteSize();
    SkDebugf("real size : %ld\n", size);

    //SkPMColor* pixels = new SkPMColor[size];
    SkPMColor* pixels = reinterpret_cast<SkPMColor*>(malloc(size));
    
    sk_sp<SkSurface> surface(SkSurfaces::WrapPixels(info, pixels, info.minRowBytes()));

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPMColor pmWhite = pixels[0];
    SkPaint paint;
    canvas->drawPoint(1, 1, paint);

    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
    //delete[] pixels;
    free(pixels);
}