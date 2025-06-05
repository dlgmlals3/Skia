#include "TestLibrary.h" // SkSurfaces::WrapBackendRenderTarget, SkImages::DeferredFromEncodedData
#include "SaveLayerTest.h" // SaveLayerTest

void TestLibrary::Draw(SkCanvas *canvas) {
    printf("dlgmlals3 Draw Images");      
    canvas->clear(SK_ColorWHITE);
}

void TestLibrary::ImageRenderDrawTest(SkCanvas *canvas) {
    printf("dlgmlals3 Draw Images");      
    canvas->clear(SK_ColorWHITE);

    // 첫 번째 이미지 로드 및 그리기 (배경)
    auto bgData = GetResourceAsData("images/ducky.jpg");
    sk_sp<SkImage> bgImage = SkImages::DeferredFromEncodedData(bgData, kPremul_SkAlphaType);
    //canvas->drawImage(bgImage, 0, 0);

    auto data = GetResourceAsData("images/dog.jpg");  
    SkPaint paint;
    paint.setAlphaf(1.0f);
    paint.setBlendMode(SkBlendMode::kSrc); 
    //paint.setBlendMode(SkBlendMode::kSrcOver); 
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data, kPremul_SkAlphaType);
    sk_sp<SkImageFilter> blurFilter = SkImageFilters::Blur(10.0f, 10.0f, nullptr);
    paint.setImageFilter(blurFilter);
    canvas->drawImage(image, 50, 50, SkSamplingOptions(), &paint);
}

void TestLibrary::RenderImageToGpuCanvas(SkCanvas* canvas, const char* imagePath) {
    std::unique_ptr<SkStreamAsset> stream(SkStream::MakeFromFile(imagePath));
    if (!stream) {
        fprintf(stderr, "Failed to open image file: %s\n", imagePath);
        return;
    }
    sk_sp<SkData> encoded = SkData::MakeFromStream(stream.get(), stream->getLength());
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(encoded, kPremul_SkAlphaType);
    if (!image) {
        printf("Failed to decode image.\n");
        return;
    }
    canvas->drawImage(image, 0, 0);
}

void TestLibrary::DrawSquare(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setStyle(SkPaint::kFill_Style);
    
    SkRect rect = SkRect::MakeXYWH(100, 100, 200, 200);
    canvas->drawRect(rect, paint);
}

void TestLibrary::SaveSkp(sk_sp<SkPicture> picture, const char* filename) {
    sk_sp<SkData> data = picture->serialize();
    if (data) {
        SkFILEWStream stream(filename);
        stream.write(data->data(), data->size());
    }
}

TestLibrary testLib;
SaveLayerTest saveLayer;
void TestLibrary::TestFunc(int width, int height, sk_sp<SkSurface> surface) {
    if (RENDERING == 1) {
        testLib.Draw(surface->getCanvas());
    } else {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(width, height));
        saveLayer.Draw(canvas);
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        testLib.SaveSkp(picture, "output.skp");
    }
}

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

    
    int width = 800;
    int height = 600;

    GrGLFramebufferInfo info;
    info.fFBOID = 0; // 기본 FBO (보통 X11, EGL 등에서)
    info.fFormat = GR_GL_RGBA8;
    int sampleCnt = 0; // No multisampling
    int stencil = 8;   // 8 bits stencil buffer

    // Skia용 BackendRenderTarget 생성
    auto target = GrBackendRenderTargets::MakeGL(width, height, sampleCnt, stencil, info);
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface) {
        fprintf(stderr, "GrGLMakeNativeInterface failed\n");
        return -1;
    }

    // Skia용 컨텍스트 생성
    sk_sp<GrDirectContext> fContext = GrDirectContexts::MakeGL(interface);
    if (!fContext) {
        fprintf(stderr, "GrDirectContext::MakeGL failed\n");
        return -1;
    }

    // Create SkSurface
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();        
    sk_sp<SkSurface> surface(SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                                target,
                                                                kBottomLeft_GrSurfaceOrigin,
                                                                kRGBA_8888_SkColorType,
                                                                colorSpace,
                                                                nullptr));

    testLib.TestFunc(width, height, surface);

    // Flush
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
    return 0;
}
