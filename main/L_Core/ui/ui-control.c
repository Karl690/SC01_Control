#include "main.h"
#include "ui.h"
#include "ui-control.h"
#include "configure.h"
#include "K_Core/pcnt/pcnt.h"
#include "K_Core/taskmanager.h"
lv_obj_t* ui_control_screen;
UI_CONTROL ui_control;
void ui_control_button_handler(lv_event_t * e) {
	lv_obj_t* obj = lv_event_get_target(e);
	systemconfig.pcnt.enabled = systemconfig.pcnt.enabled == 1 ? 0 : 1;
	
	save_configuration();
	// ui_show_messagebox(systemconfig.pcnt.enabled ? MESSAGEBOX_INFO: MESSAGEBOX_ERROR, systemconfig.pcnt.enabled ? "TEMP CTRL is enabled" : "TEMP CTRL is disabled" , 1000);
}

void ui_control_temp_ctrl_handler(lv_event_t* e) {
	int code = (int)lv_event_get_user_data(e);
	int temp = systemconfig.pcnt.programmed_temperature;
	int direction = 0;
	
	switch(code) {
	case 0:
		direction = -1;
		break;
	case 1:
		direction = 1;
		break;
	}

	temp += direction;
	if (temp < 0) temp = 0;
	else if (temp > 300) temp = 300;
	systemconfig.pcnt.programmed_temperature = temp;
	sprintf(ui_temp_string, "%d'C", systemconfig.pcnt.programmed_temperature);
	save_configuration();
	lv_label_set_text(ui_control.programmed_temp, ui_temp_string);//pcnt_info.temperature
	
}

void ui_control_refresh() {

	lv_label_set_text_fmt(ui_control.hb, "HB: %d", HeartBeat);

	ui_change_button_color(ui_control.onoff, systemconfig.pcnt.enabled == 1? 0x00ff00 : 0xff0000, 0xffffff);
	ui_change_button_text(ui_control.onoff, systemconfig.pcnt.enabled? "ON": "OFF");

	sprintf(ui_temp_string, "%.1f'C", pcnt_info.temperature);
	lv_label_set_text(ui_control.temp, ui_temp_string);//pcnt_info.temperature

	int CalcDuty = pcnt_info.duty * 6;
	if (CalcDuty > 100) CalcDuty = 100;
	sprintf(ui_temp_string, "%d%c", CalcDuty, '%');
	lv_label_set_text(ui_control.duty, ui_temp_string);

	lv_label_set_text_fmt(ui_control.raw_cnt1, "%d", pcnt_info.count01);
	lv_label_set_text_fmt(ui_control.raw_cnt2, "%d", pcnt_info.count02);

	float bat_percent = (pcnt_info.bat_volt - PCNT_BATTERY_EMPTY) / (PCNT_BATTERY_FULL - PCNT_BATTERY_EMPTY) * 100;
	if (bat_percent < 0) bat_percent = 0;
	sprintf(ui_temp_string, "%.1f%c", bat_percent, '%');
	lv_label_set_text(ui_control.battery, ui_temp_string);
	int bat_level = 10 - ceil(bat_percent / 10);
	lv_obj_t* obj;
	for(int i = 9; i >= 0; i --) {
		obj = lv_obj_get_child(ui_control.battery_panel, i);
		if (i < bat_level) {
			lv_obj_set_style_bg_color(obj, lv_color_hex(0x444444), LV_PART_MAIN);
		} else {
			lv_obj_set_style_bg_color(obj, lv_color_hex(0x00ff24), LV_PART_MAIN);
		}
	}
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
	lv_obj_set_pos(panel, 0, 45);

	lv_obj_t* obj = ui_create_label(ui_control_screen, "HB: ", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xfffffff), LV_PART_MAIN);
	lv_obj_set_pos(obj, 5, 5);
	ui_control.hb = obj;

	obj = ui_create_button(panel, systemconfig.pcnt.enabled ? "ON": "OFF", 120, 130, 10, &lv_font_montserrat_48, ui_control_button_handler, (void*)NULL);
	ui_change_button_color(obj, systemconfig.pcnt.enabled == 1? 0x00ff00 : 0xff0000, 0xffffff);
	lv_obj_set_pos(obj, 5, 5);
	ui_control.onoff = obj;

	obj = ui_create_label(panel, "TEMP", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 170, 2);
	
	obj = ui_create_label(panel, "22'C", &lv_font_montserrat_48);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x1df5fd), LV_PART_MAIN);
	lv_obj_set_pos(obj, 170, 30);
	ui_control.temp = obj;

	obj = ui_create_label(panel, "Raw CNT1", &lv_font_montserrat_14);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 170, 85);
	
	obj = ui_create_label(panel, "######", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x1df5fd), LV_PART_MAIN);
	lv_obj_set_size(obj, 100, 30);
	lv_obj_set_pos(obj, 170, 110);
	ui_control.raw_cnt1 = obj;

	obj = ui_create_label(panel, "Raw CNT2", &lv_font_montserrat_14);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 270, 85);

	obj = ui_create_label(panel, "#####", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x1df5fd), LV_PART_MAIN);
	lv_obj_set_size(obj, 60, 30);
	lv_obj_set_pos(obj, 270, 110);
	ui_control.raw_cnt2 = obj;
	
	obj = ui_create_button(panel, LV_SYMBOL_DOWN, 70, 70, 10, &lv_font_montserrat_48, ui_control_temp_ctrl_handler, (void*)0);
	ui_change_button_color(obj, 0x04c9f3, 0xffffff);
	lv_obj_set_pos(obj, 5, 150);


	obj = ui_create_label(panel, "SET TEMP", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 90, 150);
	
	obj = ui_create_label(panel, "#'c", &lv_font_montserrat_30);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x1df5fd), LV_PART_MAIN);
	lv_obj_set_pos(obj, 90, 180);
	ui_control.programmed_temp = obj;

	obj = ui_create_button(panel, LV_SYMBOL_UP, 70, 70, 10, &lv_font_montserrat_48, ui_control_temp_ctrl_handler, (void*)1);
	ui_change_button_color(obj, 0xfd1d47, 0xffffff);
	lv_obj_set_pos(obj, 180, 150);
	
	obj = ui_create_label(panel, "Duty", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 280, 150);

	obj = ui_create_label(panel, "#%", &lv_font_montserrat_30);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x1df5fd), LV_PART_MAIN);
	lv_obj_set_pos(obj, 280, 180);
	ui_control.duty = obj;

	
	obj = lv_obj_create(ui_control_screen);
	lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
	lv_obj_set_style_bg_color(obj, lv_color_hex(0x444444), LV_PART_MAIN);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_size(obj, 30, 10);
	lv_obj_set_pos(obj, 412, 70);
	lv_obj_set_style_pad_all(obj,  0, LV_PART_MAIN);


	lv_obj_t* bat_panel = lv_obj_create(ui_control_screen);
	lv_obj_set_style_border_width(bat_panel, 0, LV_PART_MAIN);
	lv_obj_set_style_bg_color(bat_panel, lv_color_hex(0x444444), LV_PART_MAIN);
	lv_obj_clear_flag(bat_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_size(bat_panel, 70, 125);
	lv_obj_set_pos(bat_panel, 390, 80);
	lv_obj_set_style_pad_all(bat_panel,  0, LV_PART_MAIN);
	ui_control.battery_panel = bat_panel;
	
	int yy = 3, step = 10, gap = 2;
	for(int i = 0; i < 10; i ++) {
		lv_obj_t* cell = lv_obj_create(bat_panel);
		lv_obj_set_style_border_width(cell, 1, LV_PART_MAIN);
		lv_obj_set_style_bg_color(cell, lv_color_hex(0x00ff24), LV_PART_MAIN);
		lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE); /// Flags
		lv_obj_set_size(cell, 65, step);
		lv_obj_set_pos(cell, 2, yy);
		yy += step+gap;
	}

	obj = ui_create_label(panel, "BAT %", &lv_font_montserrat_16);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xeeeeee), LV_PART_MAIN);
	lv_obj_set_pos(obj, 380, 150);
	
	obj = ui_create_label(panel, "75%", &lv_font_montserrat_30);
	lv_obj_set_style_text_color(obj, lv_color_hex(0x00ff24), LV_PART_MAIN);
	lv_obj_set_pos(obj, 370, 180);
	ui_control.battery = obj;

	sprintf(ui_temp_string, "%d'C", systemconfig.pcnt.programmed_temperature);
	lv_label_set_text(ui_control.programmed_temp, ui_temp_string);//pcnt_info.temperature
	// lv_timer_create(ui_control_update_timer, 500, NULL);
}