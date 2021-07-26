#include <raymath.h>
#include <raylib.h>
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
    CELL_TYPE_NONE = 0,
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
} Cell;

typedef Cell World[WORLD_WIDTH][WORLD_HEIGHT];

// for double-buffered logic
World world_b1 = {0};
World world_b2 = {0};
World *world_old = &world_b1;
#ifndef SINGLE_BUFFER
World *world_new = &world_b2;
#else
World *world_new = &world_b1;
#endif // SINGLE_BUFFER
float step_time = 0; // BASE_STEP_TIME;

static inline
void ClearWorld(World *world, size_t width, size_t height)
{
    memset(world, 0, sizeof(Cell) * width * height);
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
        (*world_old)[i][0] = (Cell) { .type = CELL_TYPE_WALL };
    for (size_t i = 0; i < WORLD_WIDTH; ++i)
        (*world_old)[i][WORLD_HEIGHT - 1] = (Cell) { .type = CELL_TYPE_WALL };
    for (size_t j = 0; j < WORLD_HEIGHT; ++j)
        (*world_old)[0][j] = (Cell) { .type = CELL_TYPE_WALL };
    for (size_t j = 0; j < WORLD_HEIGHT; ++j)
        (*world_old)[WORLD_WIDTH - 1][j] = (Cell) { .type = CELL_TYPE_WALL };

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
        "brush size", 1);
    Slider time_slider = NewSlider(
        (Vector2) { UI_PADDING, brush_slider.box.y + brush_slider.box.height + UI_PADDING },
        "sim speed", 1);

    float elapsed_time = 0;
    Cell paint_cell = (Cell) { .type = CELL_TYPE_SAND };
    bool painting = false;
    bool can_click = true;
    int brush_radius = 1;
    bool water_flows_right = true;
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        elapsed_time += dt;
        if (elapsed_time >= step_time) {
            elapsed_time = 0;

            for (size_t j = 0; j < WORLD_HEIGHT; ++j) {
                for (size_t i = 0; i < WORLD_WIDTH; ++i) {
                    const Cell *current_cell_old = &(*world_old)[i][j];
                    Cell *current_cell_new = &(*world_new)[i][j];
                    const Cell *cell_below_old = &(*world_old)[i][j+1];
                    Cell *cell_below_new = &(*world_new)[i][j+1];
                    switch (current_cell_old->type) {
                    case CELL_TYPE_NONE: break;
                    case CELL_TYPE_SAND: {
                        switch (cell_below_old->type) {
                        case CELL_TYPE_NONE: {
                            // fall through space
                            *cell_below_new = *current_cell_old;
                        } break;
                        case CELL_TYPE_WATER: {
                            // fall through water and push it above
                            *cell_below_new = *current_cell_old;
                            *current_cell_new = *cell_below_old;
                        } break;
                        default: {
                            // flow down diagonally to make dunes
                            if ((*world_old)[i+1][j+1].type <= CELL_TYPE_WATER)
                                (*world_new)[i+1][j+1] = *current_cell_old;
                            else if ((*world_old)[i-1][j+1].type <= CELL_TYPE_WATER)
                                (*world_new)[i-1][j+1] = *current_cell_old;
                            else
                                *current_cell_new = *current_cell_old;
                        } break;
                        }
                    } break;
                    case CELL_TYPE_WATER: {
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
                    } break;
                    case CELL_TYPE_OIL: {
                        *current_cell_new = *current_cell_old;
                    } break;
                    case CELL_TYPE_FACTORY: {
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
                    } break;
                    case CELL_TYPE_WALL: {
                        // walls trying to access their surroundings
                        // will result in a segfault, as they are the world borders
                        *current_cell_new = *current_cell_old;
                    } break;
                    default: {
                        TraceLog(LOG_FATAL, "no logic in place for cell type %d", current_cell_old->type);
                        CloseWindow(); exit(1);
                    } break;
                    }
                }
            }
            water_flows_right = !water_flows_right;

#ifndef SINGLE_BUFFER
            ClearWorld(world_old, WORLD_WIDTH, WORLD_HEIGHT);

            {
                // swap buffers
                World *tmp = world_old;
                world_old = world_new;
                world_new = tmp;
            }
#endif // SINGLE_BUFFER
        }

        // an alternative way to quit since raylib uses keycodes instead of keysyms
        // and using caps lock for escape via xkb options doesn't work >:(
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
                    paint_cell = (Cell) { .type = CELL_TYPE_FACTORY, .prop.cell_to_spawn = CELL_TYPE_NONE };
                    can_click = false; painting = false;
                } else if (ButtonClicked(none_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_NONE };
                    can_click = false; painting = false;
                } else {
                    painting = true; can_click = false;
                }
            }
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            painting  = false; can_click = true;
        }

        if (painting) {
            Vector2 p = GetMousePosition();
            p.x /= CELL_WIDTH;
            p.y /= CELL_HEIGHT;
            int ix = (size_t) Clamp(p.x, 1, WORLD_WIDTH - 2); // padding for walls
            int iy = (size_t) Clamp(p.y, 1, WORLD_HEIGHT - 2);
            for (int i = ix - (brush_radius - 1); i < ix + brush_radius; ++i) {
                if (i < 1 || i > (int) WORLD_WIDTH - 2) continue;
                for (int j = iy - (brush_radius - 1); j < iy + brush_radius; ++j) {
                    if (j < 1 || j > (int) WORLD_HEIGHT - 2) continue;
                    (*world_old)[i][j] = paint_cell;
                }
            }
        }

        BeginDrawing();
            // the "old" buffer is actually the new one due to the swap
            // I just wanted the drawing to be the very last thing in the main loop
            ClearBackground(BG_COLOR);
            for (size_t i = 0; i < WORLD_WIDTH; ++i) {
                for (size_t j = 0; j < WORLD_HEIGHT; ++j) {
                    Color col;
                    switch ((*world_old)[i][j].type) {
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
                        .x = ((float) i * CELL_WIDTH),
                        .y = ((float) j * CELL_HEIGHT),
                        .width  = CELL_WIDTH,
                        .height = CELL_HEIGHT,
                    };
                    DrawRectangleRec(rec, col);
                }
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
