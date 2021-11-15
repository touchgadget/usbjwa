#pragma once
const int MAX_ROWS = 4;
const int MAX_COLS = 8;

// Set member alignment to 1. No pad bytes between members.
#pragma pack(push, 1)
typedef struct {
    const char *cell_name;
    uint8_t joystick_control;
} Touch_Cell_t;

typedef const struct {
    uint8_t rows;
    uint8_t cols;
    const Touch_Cell_t *touch_cells;
} Touch_Page_t;

#include "joystick_esp32.h"

const Touch_Cell_t Joystick_Cells[MAX_ROWS][MAX_COLS] = {
    // Row 0, top row
    {
        {"1", 0},
        {"2", 1},
        {"3", 2},
        {"4", 3},
        {"5", 4},
        {"6", 5},
        {"7", 6},
        {"8", 7},
    },
    // Row 1
    {
        {"9", 8},
        {"10", 9},
        {"11", 10},
        {"12", 11},
        {"13", 12},
        {"14", 13},
        {"15", 14},
        {"16", 15},
    },
    // Row 2
    {
        {"17", 16},
        {"18", 17},
        {"19", 18},
        {"20", 19},
        {"21", 20},
        {"22", 21},
        {"23", 22},
        {"24", 23},
    },
    // Row 3
    {
        {"25", 24},
        {"26", 25},
        {"27", 26},
        {"28", 27},
        {"29", 28},
        {"30", 29},
        {"31", 30},
        {"32", 31},
    },
};

Touch_Page_t Joystick_Page = {
    MAX_ROWS, MAX_COLS, (const Touch_Cell_t *)Joystick_Cells
};

void Json_touch_grid(uint8_t client, Touch_Page_t &page)
{
  // Build remote control grid for web socket client. Encoded in JSON format.
  String json = "[";
  for (int r = 0; r < page.rows; r++) {
    json.concat('[');
    for (int c = 0; c < page.cols; c++) {
      Touch_Cell_t *cell = (Touch_Cell_t *)page.touch_cells + ((r * page.cols) + c);
      if (cell->cell_name != NULL) {
        DBG_printf("c = %d, r = %d, cell_name = %s\n", c, r, cell->cell_name);
        if (c != 0) json.concat(',');
        json.concat("{\"cellLabel\":\"");
        json.concat(cell->cell_name);
        json.concat('"');
        json.concat('}');
      }
    }
    if (r == (page.rows - 1)) {
      json.concat(']');
    }
    else {
      json.concat("],");
    }
  }
  json.concat(']');
  DBG_println(json);
  webSocket.sendTXT(client, json.c_str(), json.length());
}

#pragma pack(pop)
