#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <raylib.h>

typedef struct {
    Rectangle box;
    const char *text;
} Button;

Button NewButton(Vector2 pos, const char *text);
bool ButtonClicked(Button b);
void DrawButton(Button b);

typedef struct {
    Rectangle box;
    const char *text;
    float val;
} Slider;

Slider NewSlider(Vector2 pos, const char * text, float val);
bool SliderSlid(Slider *s);
void DrawSlider(Slider s);

#endif // UI_H
