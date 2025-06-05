#ifndef SaveLayerTest_h
#define SaveLayerTest_h

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
#include "include/core/SkPictureRecorder.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/effects/SkImageFilters.h"

#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"
#include "src/core/SkStreamPriv.h"
#include "tools/Resources.h" // GetResourceAsData

#define RENDERING 1

class TestLibrary {
public:
    void SaveSkp(sk_sp<SkPicture> picture, const char* filename);
    void Draw(SkCanvas *canvas);
    void ImageRenderDrawTest(SkCanvas *canvas);
    void RenderImageToGpuCanvas(SkCanvas* canvas, const char* imagePath);
    void DrawSquare(SkCanvas* canvas);
    void TestFunc(int width, int height, sk_sp<SkSurface> surface);
};
#endif