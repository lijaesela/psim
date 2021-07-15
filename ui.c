#include <stdbool.h>
#include <raylib.h>
#include "./config.h"
#include "./ui.h"

bool UI_Button(int x, int y, const char *text)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, UI_FONT_SIZE, UI_FONT_SPACING);
    Rectangle rec = {
        .x = x,
        .y = y,
        .width  = size.x,
        .height = size.y,
    };
    DrawTextRec(GetFontDefault(), text, rec, UI_FONT_SIZE, UI_FONT_SPACING, true, UI_FONT_COLOR);

    return CheckCollisionPointRec(GetMousePosition(), rec)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

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
    return CheckCollisionPointRec(GetMousePosition(), b.box);
}

void DrawButton(Button b)
{
    DrawRectangleRec(b.box, UI_BUTTON_COLOR);
    DrawTextEx(GetFontDefault(), b.text, (Vector2) { b.box.x, b.box.y }, UI_FONT_SIZE, UI_FONT_SPACING, UI_FONT_COLOR);
}
