#include <stdbool.h>
#include <raylib.h>
#include "./config.h"
#include "./ui.h"

Button NewButton(Vector2 pos, const char *text)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, UI_FONT_SIZE, UI_FONT_SPACING);
    return (Button) {
        .box = (Rectangle) {
            .x = pos.x,
            .y = pos.y,
            .width  = size.x,
            .height = size.y,
        },
        .text = text,
    };
}

bool ButtonClicked(Button b)
{
    // NOTE: this only checks collision.
    // this must be wrapped in a check for the mouse button being pressed.
    // this is for efficiency.
    return CheckCollisionPointRec(GetMousePosition(), b.box);
}

void DrawButton(Button b)
{
    DrawRectangleRec(b.box, UI_BUTTON_COLOR);
    DrawTextEx(GetFontDefault(), b.text, (Vector2) { b.box.x, b.box.y }, UI_FONT_SIZE, UI_FONT_SPACING, UI_FONT_COLOR);
}

Slider NewSlider(Vector2 pos, const char * text, float val)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, UI_FONT_SIZE, UI_FONT_SPACING);
    return (Slider) {
        .box = (Rectangle) {
            .x = pos.x,
            .y = pos.y,
            .width  = size.x,
            .height = size.y,
        },
        .text = text,
        .val = val,
    };
}

bool SliderSlid(Slider *s) {
    Vector2 mouse_pos = GetMousePosition();
    if (CheckCollisionPointRec(mouse_pos, s->box)) {
        s->val = (mouse_pos.x - s->box.x) / s->box.width;
        return true;
    }

    return false;
}

void DrawSlider(Slider s) {
    DrawRectangleRec(s.box, UI_BUTTON_COLOR);
    float line_distance = s.box.x + (s.val * s.box.width);
    DrawLineEx((Vector2) { line_distance, s.box.y },
               (Vector2) { line_distance, s.box.y + s.box.height },
               UI_LINE_THICC, UI_SLIDER_BAR_COLOR);
    DrawTextEx(GetFontDefault(), s.text,
               (Vector2) { s.box.x, s.box.y },
               UI_FONT_SIZE, UI_FONT_SPACING, UI_FONT_COLOR);
}
