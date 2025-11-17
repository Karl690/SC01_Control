#include "main.h"
#include "ui.h"
#include "ui-control.h"
#include "configure.h"
lv_obj_t* ui_control_screen;

void ui_control_button_handler(lv_event_t * e) {
//	lv_event_code_t event_code = lv_event_get_code(e); 
//	lv_obj_t * target = lv_event_get_target(e);	
}

void ui_control_screen_init()
{
	ui_control_screen = ui_create_screen();	
	ui_create_pct_title(ui_control_screen, SCREEN_CONTROLS);
	
	lv_obj_t* title_label = lv_label_create(ui_control_screen);	
	lv_obj_set_width(title_label, LV_SIZE_CONTENT);
	lv_obj_set_height(title_label, LV_SIZE_CONTENT);
	lv_label_set_recolor(title_label, true);
	lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_label_set_text(title_label, "HYREL CONTROL");
	lv_obj_set_style_text_font(title_label, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 5);
	
	lv_obj_t* panel = lv_obj_create(ui_control_screen);
	lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
	lv_obj_set_size(panel, 480, 320-55);
	lv_obj_set_style_bg_color(panel, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_pos(panel, 0, 55);

	lv_obj_t* obj = ui_create_button(panel, systemconfig.pcnt.enabled ? "ON": "OFF", 120, 80, 10, &lv_font_montserrat_48, ui_control_button_handler, (void*)NULL);
	ui_change_button_color(obj, systemconfig.pcnt.enabled == 1? 0x00ff00 : 0xff0000, 0xffffff);
	lv_obj_set_pos(obj, 5, 5);

	obj = ui_create_label(panel, "TEMP", &lv_font_montserrat_20);
	lv_obj_set_pos(obj, 200, 5);
	
	obj = ui_create_label(panel, "22'C", &lv_font_montserrat_48);
	lv_obj_set_pos(obj, 200, 35);

	obj = ui_create_button(panel, LV_SYMBOL_DOWN, 80, 80, 10, &lv_font_montserrat_48, ui_control_button_handler, (void*)NULL);
	ui_change_button_color(obj, UI_BUTTON_NORMAL_BG_COLOR, 0xffffff);
	lv_obj_set_pos(obj, 5, 150);


	obj = ui_create_label(panel, "SET TEMP", &lv_font_montserrat_20);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_set_pos(obj, 5, 150);

	obj = ui_create_button(panel, LV_SYMBOL_UP, 80, 80, 10, &lv_font_montserrat_48, ui_control_button_handler, (void*)NULL);
	ui_change_button_color(obj, UI_BUTTON_NORMAL_BG_COLOR, 0xffffff);
	lv_obj_set_pos(obj, 200, 170);
	
}