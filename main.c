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

#define PRINT_POS(pos) \
    printf(#pos": {\n    x: %zu\n    y: %zu\n}\n", \
           (pos).x, (pos).y);
#define PRINT_REC(rec) \
    printf(#rec": {\n    x: %f\n    y: %f\n    w: %f\n    h: %f\n}\n", \
           (rec).x, (rec).y, (rec).width, (rec).height);

//#define SINGLE_BUFFER

typedef enum {
    CELL_TYPE_NONE = 0,
    CELL_TYPE_SAND,
    CELL_TYPE_WATER,
    CELL_TYPE_WALL,
} CellType;

typedef union {
    int health;
} Property;

typedef struct {
    CellType type;
    //Property prop;
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

typedef struct {
    size_t x;
    size_t y;
} Pos;

Pos WindowPosToWorldPos(Vector2 window_pos)
{
    return (Pos) {
        .x = window_pos.x / CELL_WIDTH,
        .y = window_pos.y / CELL_HEIGHT,
    };
}

void ClearWorld(World *world, size_t width, size_t height)
{
    memset(world, 0, sizeof(Cell) * WORLD_WIDTH * WORLD_HEIGHT);
    /*
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            (*world)[i][j] = (Cell) { .type = CELL_TYPE_NONE };
        }
    }
    */
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
    Slider time_slider = NewSlider(
        (Vector2) { UI_PADDING, water_button.box.y + water_button.box.height + UI_PADDING },
        "sim speed", 1);

    float elapsed_time = 0;
    Cell paint_cell = (Cell) { .type = CELL_TYPE_SAND };
    bool painting = false;
    bool can_click = true;
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
                    //printf("%zu, %zu: switching on: %d\n", i, j, current_cell_old->type);
                    switch (current_cell_old->type) {
                    case CELL_TYPE_NONE: break;
                    case CELL_TYPE_SAND: {
                        size_t target_x = i;
                        size_t target_y = j + 1;
                        const Cell *target_cell_old = &(*world_old)[target_x][target_y];
                        Cell *target_cell_new = &(*world_new)[target_x][target_y];
                        switch (target_cell_old->type) {
                        case CELL_TYPE_NONE: {
                            *target_cell_new = *current_cell_old;
                        } break;
                        default: {
                            if ((*world_old)[i+1][j+1].type == CELL_TYPE_NONE)
                                (*world_new)[i+1][j+1] = *current_cell_old;
                            else if ((*world_old)[i-1][j+1].type == CELL_TYPE_NONE)
                                (*world_new)[i-1][j+1] = *current_cell_old;
                            else
                                *current_cell_new = *current_cell_old;
                        } break;
                        }
                    } break;
                    case CELL_TYPE_WATER: {
                        size_t target_x = i;
                        size_t target_y = j + 1;
                        const Cell *target_cell_old = &(*world_old)[target_x][target_y];
                        Cell *target_cell_new = &(*world_new)[target_x][target_y];
                        switch (target_cell_old->type) {
                        case CELL_TYPE_NONE: {
                            *target_cell_new = *current_cell_old;
                        } break;
                        default: {
                            bool moved = true;
                            if (GetRandomValue(0, 1)) {
                                if ((*world_old)[i+1][j+1].type == CELL_TYPE_NONE && (*world_new)[i+1][j+1].type == CELL_TYPE_NONE)
                                    (*world_new)[i+1][j+1] = *current_cell_old;
                                else  if ((*world_old)[i+1][j].type == CELL_TYPE_NONE && (*world_new)[i+1][j].type == CELL_TYPE_NONE)
                                    (*world_new)[i+1][j] = *current_cell_old;
                                else
                                    moved = false;
                            } else {
                                if ((*world_old)[i-1][j+1].type == CELL_TYPE_NONE && (*world_new)[i-1][j+1].type == CELL_TYPE_NONE)
                                    (*world_new)[i-1][j+1] = *current_cell_old;
                                else if ((*world_old)[i-1][j].type == CELL_TYPE_NONE && (*world_new)[i-1][j].type == CELL_TYPE_NONE)
                                    (*world_new)[i-1][j] = *current_cell_old;
                                else
                                    moved = false;
                            }
                            if (!moved)
                                *current_cell_new = *current_cell_old;
                        } break;
                        }
                    } break;
                    case CELL_TYPE_WALL: {
                        // walls trying to access their surroundings
                        // will result in a segfault
                        (*world_new)[i][j] = (Cell) { .type = CELL_TYPE_WALL };
                    } break;
                    default: assert(false && "unreachable");
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

        // mmm yum boolean soup
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            painting = false;
            /* holdable things */
            if (SliderSlid(&time_slider)) {
                step_time = BASE_STEP_TIME - time_slider.val * BASE_STEP_TIME;
                can_click = false;
            } else if (can_click) {
                /* non-holdable, single click */
                if (ButtonClicked(sand_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_SAND };
                    can_click = false;
                } else if (ButtonClicked(water_button)) {
                    paint_cell = (Cell) { .type = CELL_TYPE_WATER };
                    can_click = false;
                } else {
                    painting = true;
                }
            }
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            painting  = false;
            can_click = true;
        }

        if (painting) {
            Pos p = WindowPosToWorldPos(GetMousePosition());
            if (p.x > 0 && p.y > 0 && p.x < WORLD_WIDTH - 1 && p.y < WORLD_HEIGHT - 1) {
                (*world_old)[p.x][p.y] = paint_cell;
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
                    case CELL_TYPE_SAND:
                        col = SAND_COLOR;
                        break;
                    case CELL_TYPE_WATER:
                        col = WATER_COLOR;
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
            DrawSlider(time_slider);

        EndDrawing();
    }

    CloseWindow();
}
