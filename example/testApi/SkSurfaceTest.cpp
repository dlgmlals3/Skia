#include "../include/SkSurfaceTest.h"

void SkSurfaceTestDraw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    canvas->drawLine(20, 100, 100, 100, p);
}