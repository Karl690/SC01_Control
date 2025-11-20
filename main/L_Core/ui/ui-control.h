#pragma once
#include "ui.h"
typedef enum
{
	CONTROLS_ONOFF,
	CONTROLS_TEMP_DECREASE,
	CONTROLS_TEMP_INCREASE,
}CONTROLS_BUTTON_LIST;

typedef struct _tagUiControl {
	lv_obj_t* hb;
	lv_obj_t* onoff;
	lv_obj_t* temp;
	lv_obj_t* programmed_temp;
	lv_obj_t* duty;
	lv_obj_t* battery;
	lv_obj_t* battery_panel;
	lv_obj_t* raw_cnt1;
	lv_obj_t* raw_cnt2;
} UI_CONTROL;

extern lv_obj_t* ui_control_screen;
void ui_control_screen_init();
void ui_control_refresh();

