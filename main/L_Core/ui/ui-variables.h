#pragma once
#include "lvgl.h"

typedef struct {
    lv_obj_t* panel;
    lv_obj_t* title;
    lv_obj_t* value;
    void* data_item;
} UI_VARIABLES_ITEM;


extern lv_obj_t* ui_variable_screen;

void ui_variables_screen_init(void);
void ui_variables_refresh();