#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "./config.h"
#include "./ui.h"

#define CELL_WIDTH  ((float) WIN_WIDTH  / (float) WORLD_WIDTH)
#define CELL_HEIGHT ((float) WIN_HEIGHT / (float) WORLD_HEIGHT)
#define RANDOM_BOOL GetRandomValue(0, 1)

#define PRINT_POS(pos) \
    printf(#pos": {\n    x: %zu\n    y: %zu\n}\n", \
           (pos).x, (pos).y);
#define PRINT_VEC(vec) \
    printf(#vec": {\n    x: %f\n    y: %f\n}\n", \
           (vec).x, (vec).y);
#define PRINT_REC(rec) \
    printf(#rec": {\n    x: %f\n    y: %f\n    w: %f\n    h: %f\n}\n", \
           (rec).x, (rec).y, (rec).width, (rec).height);

// NOTE: MUST be in order of weight
typedef enum {
    //CELL_TYPE_NONE = 0, // no more!
    CELL_TYPE_FIRE,
    CELL_TYPE_OIL,
    CELL_TYPE_WATER,
    CELL_TYPE_SAND,
    CELL_TYPE_FACTORY,
    CELL_TYPE_WALL,
    COUNT_CELL_TYPES,
} CellType;

typedef union {
    CellType cell_to_spawn;
} CellProp;

typedef struct {
    CellType type;
    CellProp prop;
    int x;
    int y;
} Cell;

#define MAX_CELLS (WORLD_WIDTH * WORLD_HEIGHT)

Cell CELLS[MAX_CELLS];
size_t CELL_COUNT = 0;

float step_time = 0;

void add_cell(Cell c)
{
    if (CELL_COUNT != MAX_CELLS)
        CELLS[CELL_COUNT++] = c;
    else
        TraceLog(LOG_WARNING, "cell limit of %zu reached");
}

size_t ClampSize(size_t val, size_t min, size_t max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

int main(void)
{
    SetTargetFPS(FPS);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, TITLE);

    /* immortal borders */
    for (size_t i = 0; i < WORLD_WIDTH; ++i)
        add_cell((Cell) { .type = CELL_TYPE_WALL, .x = i, .y = 0});
    for (size_t i = 0; i < WORLD_WIDTH; ++i)
        add_cell((Cell) { .type = CELL_TYPE_WALL, .x = i, .y = WORLD_HEIGHT - 1});
    for (size_t j = 1; j < WORLD_HEIGHT - 1; ++j)
        add_cell((Cell) { .type = CELL_TYPE_WALL, .x = 0, .y = j});
    for (size_t j = 1; j < WORLD_HEIGHT - 1; ++j)
        add_cell((Cell) { .type = CELL_TYPE_WALL, .x = WORLD_WIDTH - 1, .y = j});

    /* initiate UI */
    Button sand_button = NewButton(
        (Vector2) { UI_PADDING, UI_PADDING },
        "sand");
    Button water_button = NewButton(
        (Vector2) { UI_PADDING, sand_button.box.y + sand_button.box.height + UI_PADDING },
        "water");
    Button oil_button = NewButton(
        (Vector2) { UI_PADDING, water_button.box.y + water_button.box.height + UI_PADDING },
        "oil");
    Button wall_button = NewButton(
        (Vector2) { UI_PADDING, oil_button.box.y + oil_button.box.height + UI_PADDING },
        "wall");
    Button factory_button = NewButton(
        (Vector2) { UI_PADDING, wall_button.box.y + wall_button.box.height + UI_PADDING },
        "factory");
    Button none_button = NewButton(
        (Vector2) { UI_PADDING, factory_button.box.y + factory_button.box.height + UI_PADDING },
        "none (erase)");
    Slider brush_slider = NewSlider(
        (Vector2) { UI_PADDING, none_button.box.y + none_button.box.height + UI_PADDING },
        "brush size", 0);
    Slider time_slider = NewSlider(
        (Vector2) { UI_PADDING, brush_slider.box.y + brush_slider.box.height + UI_PADDING },
        "sim speed", 1);

    float elapsed_time = 0;
    Cell paint_cell = (Cell) { .type = CELL_TYPE_SAND };
    bool painting = false;
    bool can_click = true;
    int brush_radius = BRUSH_MIN;
    bool water_flows_right = true;
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        elapsed_time += dt;
        if (elapsed_time >= step_time) {
            elapsed_time = 0;

            for (size_t i = 0; i < CELL_COUNT; ++i) {
                switch (CELLS[i].type) {
                case CELL_TYPE_SAND: {
                    bool can_fall = true;
                    for (size_t j = 0; j < CELL_COUNT; ++j) {
                        if (CELLS[j].x == CELLS[i].x
                            && CELLS[j].y == CELLS[i].y + 1)
                            can_fall = false;
                    }
                    if (can_fall) CELLS[i].y++;
                } break;
                case CELL_TYPE_WATER: {
                    /*
                    switch (cell_below_old->type) {
                    case CELL_TYPE_NONE: {
                        // fall through space
                        *cell_below_new = *current_cell_old;
                    } break;
                    default: {
                        // fill!
                        bool moved = true;
                        if (RANDOM_BOOL) {
                            if ((*world_old)[i+1][j].type == CELL_TYPE_NONE
                                && (*world_new)[i+1][j].type == CELL_TYPE_NONE) {
                                if ((*world_old)[i+2][j].type == CELL_TYPE_NONE
                                    && (*world_new)[i+2][j].type == CELL_TYPE_NONE)
                                    (*world_new)[i+2][j] = *current_cell_old;
                                else
                                    (*world_new)[i+1][j] = *current_cell_old;
                            } else
                                moved = false;
                        } else {
                            if ((*world_old)[i-1][j].type == CELL_TYPE_NONE
                                && (*world_new)[i-1][j].type == CELL_TYPE_NONE) {
                                if ((*world_old)[i-2][j].type == CELL_TYPE_NONE
                                    && (*world_new)[i-2][j].type == CELL_TYPE_NONE)
                                    (*world_new)[i-2][j] = *current_cell_old;
                                else
                                    (*world_new)[i-1][j] = *current_cell_old;
                            } else
                                moved = false;
                        }
                        if (!moved && current_cell_new->type == CELL_TYPE_NONE)
                            *current_cell_new = *current_cell_old;
                    } break;
                    }
                    */
                } break;
                case CELL_TYPE_OIL: {
                } break;
                case CELL_TYPE_FACTORY: {
                    /*
                    const Cell *cell_above_old = &(*world_old)[i][j-1];
                    if (current_cell_old->prop.cell_to_spawn != CELL_TYPE_NONE
                        && cell_below_old->type == CELL_TYPE_NONE
                        && cell_below_new->type == CELL_TYPE_NONE)
                        *cell_below_new = (Cell) {
                            .type = current_cell_old->prop.cell_to_spawn,
                                .prop = {0},
                        };
                    if (cell_above_old->type != CELL_TYPE_NONE)
                        *current_cell_new = (Cell) {
                            .type = CELL_TYPE_FACTORY,
                                .prop.cell_to_spawn = cell_above_old->type,
                        };
                    else
                        *current_cell_new = *current_cell_old;
                        */
                } break;
                case CELL_TYPE_WALL: {
                } break;
                default: {
                    TraceLog(LOG_FATAL, "no logic in place for cell type %d", CELLS[i].type);
                    CloseWindow(); exit(1);
                } break;
                }

                if (CELLS[i].type != CELL_TYPE_WALL) {
                    if /**/ (CELLS[i].x >= (int) WORLD_WIDTH)
                        CELLS[i].x = 0;
                    else if (CELLS[i].x < 0)
                        CELLS[i].x = WORLD_WIDTH - 1;
                    else if (CELLS[i].y >= (int) WORLD_HEIGHT)
                        CELLS[i].y = 0;
                    else if (CELLS[i].y < 0)
                        CELLS[i].y = WORLD_HEIGHT - 1;
                }
            }
            water_flows_right = !water_flows_right;
        }

        // an alternative way to quit since raylib uses keycodes instead of keysyms
        // and using caps lock for escape via xkb options doesn't work >:(
        // TODO: I can change the quit key with raylib.
        if (IsKeyDown(KEY_Q)) {
            CloseWindow();
            exit(0);
        }

        // mmm yum boolean soup
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            /* holdable things */
            if (!painting) {
                if (SliderSlid(&time_slider)) {
                    step_time = BASE_STEP_TIME - time_slider.val * BASE_STEP_TIME;
                    can_click = false; painting = false;
                } else if (SliderSlid(&brush_slider)) {
                    brush_radius = (int) (brush_slider.val * (BRUSH_MAX - BRUSH_MIN)) + BRUSH_MIN;
                    can_click = false; painting = false;
                }
            }
            if (can_click) {
                /* non-holdable, single click */
                if (ButtonClicked(sand_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_SAND };
                    can_click = false; painting = false;
                } else if (ButtonClicked(water_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_WATER };
                    can_click = false; painting = false;
                } else if (ButtonClicked(oil_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_OIL };
                    can_click = false; painting = false;
                } else if (ButtonClicked(wall_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_WALL };
                    can_click = false; painting = false;
                } else if (ButtonClicked(factory_button)) {
                    // TODO: look, factories must be reworked
                    // paint_cell = (Cell) { .type = CELL_TYPE_FACTORY, .prop.cell_to_spawn = CELL_TYPE_NONE };
                    can_click = false; painting = false;
                } else if (ButtonClicked(none_button)) {
                    // TODO: look, erasing must be reworked
                    //paint_cell = (Cell) { .type = CELL_TYPE_NONE };
                    can_click = false; painting = false;
                } else {
                    painting = true; can_click = false;
                }
            }
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            painting  = false; can_click = true;
        }

        // TODO: painting existing outside of FPS regulation may be bad
        if (painting) {
            Vector2 p = GetMousePosition();
            p.x /= CELL_WIDTH;
            p.y /= CELL_HEIGHT;
            int ix = (size_t) Clamp(p.x, 1, WORLD_WIDTH - 2); // padding for walls
            int iy = (size_t) Clamp(p.y, 1, WORLD_HEIGHT - 2);
            // loop for brush size
            for (int i = ix - (brush_radius - 1); i < ix + brush_radius; ++i) {
                if (i < 1 || i > (int) WORLD_WIDTH - 2) continue;
                for (int j = iy - (brush_radius - 1); j < iy + brush_radius; ++j) {
                    if (j < 1 || j > (int) WORLD_HEIGHT - 2) continue;
                    // TODO: check if a cell exists under the cursor
                    paint_cell.x = i;
                    paint_cell.y = j;
                    add_cell(paint_cell);
                }
            }
        }

        BeginDrawing();
            // the "old" buffer is actually the new one due to the swap
            // I just wanted the drawing to be the very last thing in the main loop
            ClearBackground(BG_COLOR);
            for (size_t i = 0; i < CELL_COUNT; ++i) {
                Color col;
                switch (CELLS[i].type) {
                case CELL_TYPE_WALL:
                    col = WALL_COLOR;
                    break;
                case CELL_TYPE_FACTORY:
                    col = FACTORY_COLOR;
                    break;
                case CELL_TYPE_SAND:
                    col = SAND_COLOR;
                    break;
                case CELL_TYPE_WATER:
                    col = WATER_COLOR;
                    break;
                case CELL_TYPE_OIL:
                    col = OIL_COLOR;
                    break;
                default:
                    col = BG_COLOR;
                    break;
                }
                Rectangle rec = {
                    .x = ((float) CELLS[i].x * CELL_WIDTH),
                    .y = ((float) CELLS[i].y * CELL_HEIGHT),
                    .width  = CELL_WIDTH,
                    .height = CELL_HEIGHT,
                };
                DrawRectangleRec(rec, col);
            }

            DrawButton(sand_button);
            DrawButton(water_button);
            DrawButton(oil_button);
            DrawButton(wall_button);
            DrawButton(factory_button);
            DrawButton(none_button);
            DrawSlider(brush_slider);
            DrawSlider(time_slider);

        EndDrawing();
    }

    CloseWindow();
}
