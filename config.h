#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>

#define FPS 60
#define TITLE "particle simulation"
#define WIN_WIDTH  640u
#define WIN_HEIGHT 480u

#define UI_PADDING 5
#define UI_FONT_SIZE    20
#define UI_FONT_SPACING 2
#define UI_FONT_COLOR   BLACK
#define UI_BUTTON_PADDING 10
#define UI_BUTTON_COLOR YELLOW
#define UI_LINE_THICC   5
#define UI_SLIDER_BAR_COLOR GREEN

#define BG_COLOR    RAYWHITE
#define SAND_COLOR  BEIGE
#define WALL_COLOR  GRAY
#define WATER_COLOR BLUE

#define WORLD_WIDTH  64u
#define WORLD_HEIGHT 48u
#define BASE_STEP_TIME 0.4f

#endif // CONFIG_H
