#include "TestLibrary.h"

class SaveLayerTest {
public:
    // 레이어를 사용한 배경 흐림 처리 및 자식 뷰 렌더링 테스트
    // 22 라인을 canvas->restore() 34라인에 넣게 되면 
    // 하나의 프레임버퍼에 그려지기 때문에 전체 화면이 흐려지게 된다.
    void Draw(SkCanvas *canvas) {
        printf("dlgmlals3 SaveLayerTest");
        canvas->clear(SK_ColorBLACK);
        // 최상위 레이어 (배경 흐림 처리용) ====
        SkPaint blurLayerPaint;
        blurLayerPaint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));
        canvas->saveLayer(nullptr, &blurLayerPaint);
        {
            // 새로운 레이어로 배경 그리기
            auto bgData = GetResourceAsData("images/ducky.jpg");
            sk_sp<SkImage> bgImage = SkImages::DeferredFromEncodedData(bgData, kPremul_SkAlphaType);
            // Clipping 적용
            canvas->clipRect(SkRect::MakeXYWH(0, 0, 200, 200));        
            canvas->drawImage(bgImage, 0, 0);
        }
        canvas->restore();
        
        // 중첩 레이어 사각형 (알파 적용)
        SkPaint alphaLayerPaint;
        alphaLayerPaint.setAlpha(255);
        canvas->saveLayer(nullptr, &alphaLayerPaint);
        {
            // 자식 뷰 렌더링
            SkPaint paint;
            paint.setColor(SK_ColorBLUE);
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), paint);
        }
        canvas->restore();    
    }

    // docker cp d444d342a10f:/workspace/Skia/skp/saveLayerTest.skp /Share/
    void DrawHWLayer(SkCanvas *canvas) {
        printf("dlgmlals3 DrawHWLayer Test\n");
        auto bgData = GetResourceAsData("images/ducky.jpg");
        auto bgImage = SkImages::DeferredFromEncodedData(bgData, kPremul_SkAlphaType);
        auto dogData = GetResourceAsData("images/dog.jpg");
        auto dogImage = SkImages::DeferredFromEncodedData(dogData, kPremul_SkAlphaType);
        // 즉시 디코딩함. 이거 빼먹으면 gpu안나오는 경우 있음.
        dogImage = dogImage->makeRasterImage();

        canvas->clear(SK_ColorBLACK);
        
        canvas->drawAnnotation(SkRect::MakeXYWH(100, 0, 150, 150), "OffscreenComposite", nullptr);

        // 서피스 생성
        SkPaint paint;        
        auto dogSurface = TestLibrary::CreateSurface(150, 150, 1);
        SkCanvas* dogCanvas = dogSurface->getCanvas();
        dogCanvas->clear(SK_ColorGRAY);        
        dogCanvas->drawImage(dogImage, 0, 0);
               
        sk_sp<SkImage> dogSnap = dogSurface->makeImageSnapshot();
        canvas->drawImage(bgImage, 0, 0);
        canvas->drawImage(dogSnap, 100, 0);
        //TestLibrary::flush();
    }

    
    void drawFont(SkCanvas *canvas) {
        SkPaint paint;        
        paint.setColor(SK_ColorBLUE);
        paint.setAlpha(255);      

        sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefEmpty();
        const char* fontFamily = "Arial";
        sk_sp<SkTypeface> typeface = fontMgr->matchFamilyStyle("Arial", SkFontStyle::Normal());
        if (!typeface) {
            printf("Font '%s' not found, falling back to default\n", fontFamily);
            typeface = fontMgr->legacyMakeTypeface(nullptr, SkFontStyle());  // fallback
        }
        printf("fonnt count : %d\n", fontMgr->countFamilies());
        for (int i = 0; i < fontMgr->countFamilies(); ++i) {
            SkString name;
            fontMgr->getFamilyName(i, &name);
            printf("Font %d: %s\n", i, name.c_str());
        }
        
        SkFont font(typeface, 100);     
        canvas->drawString("Hello, Skia!", 0, 0, font, paint);
    }
};