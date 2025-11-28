#include "ui.h"
#include "ui-variables.h"
#include "K_Core/display/DisplayList.h"

#define UI_VARIABLES_ITEM_SIZE 8

lv_obj_t* ui_variable_screen;


UI_VARIABLES_ITEM ui_variables_ui_items[UI_VARIABLES_ITEM_SIZE];

void ui_variables_button_callback(lv_event_t* e) {
	int code = (int)lv_event_get_user_data(e);
	switch(code) {
	case 0: // previus
		ShowPreviousDisplay();
		break;
	case 1: // Next
		ShowNextDisplay();
		break;
	}
}

void ui_variables_refresh()
{
	if (!lv_obj_is_visible(ui_variable_screen)) return;
	DisplayVariableInfo* activeDisplayVarInfo = GetActiveDisplay();;
	for (int i = 0; i < UI_VARIABLES_ITEM_SIZE; i ++) {
		UI_VARIABLES_ITEM	* item = &ui_variables_ui_items[i];
		if (!activeDisplayVarInfo[i].VariablePointer) {
			item->data_item = NULL;
			lv_label_set_text(item->title, "");
			lv_label_set_text(item->value, "");
		} else {
			DisplayVariableInfo* info = &activeDisplayVarInfo[i];
			item->data_item = info;
			lv_label_set_text(item->title, info->Label);
			// if (strcmp(info->Label, "HB") == 0) {
			// 	ESP_LOGI(TAG, "HeartBeat: %d %d", *(int*)info->VariablePointer, HeartBeat);
			// }

			switch (info->FuncType)
			{
			case FUNC_INT32:
				lv_label_set_text_fmt(item->value, "%d", *(int*)info->VariablePointer);
				break;
			case FUNC_FLOAT:
				sprintf(ui_temp_string, "%.2f", *(float*)info->VariablePointer);
				lv_label_set_text(item->value, ui_temp_string);
				break;
			case FUNC_BOOLEAN:
				lv_label_set_text_fmt(item->value, "%s", *(bool*)info->VariablePointer == 1? "ON": "OFF");
				break;
			default:
				lv_label_set_text(item->value, (char*)info->VariablePointer);
				break;
			}
		}
	}
}

void ui_variables_screen_init(void)
{	
	ui_variable_screen = ui_create_screen();	
	ui_create_pct_title(ui_variable_screen, SCREEN_VARIALBES);
	
	lv_obj_t* obj = lv_label_create(ui_variable_screen);	
	lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);	
	lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_label_set_text(obj, "VARIABLES");
	lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 100, 5);

	obj = ui_create_button(ui_variable_screen, LV_SYMBOL_LEFT, 40, 40, 3, &lv_font_montserrat_14, ui_variables_button_callback, (void*)0);
	lv_obj_set_pos(obj, 5, 5);

	obj = ui_create_button(ui_variable_screen, LV_SYMBOL_RIGHT, 40, 40, 3, &lv_font_montserrat_14, ui_variables_button_callback, (void*)1);
	lv_obj_set_pos(obj, 340, 5);

	int x = 5, y = 5, gap = 5;

	lv_obj_t* container = ui_helpers_create_panel(ui_variable_screen, 0x00000, false);
	lv_obj_set_size(container, 470, 256);
	lv_obj_set_pos(container, 5, 60);
	
	lv_obj_t* panel;
	int step = 27;
	for (uint8_t i = 0; i < UI_VARIABLES_ITEM_SIZE; i++) {
		UI_VARIABLES_ITEM *ui_item = &ui_variables_ui_items[i];
		panel = ui_helpers_create_panel(container, 0x0, false);
		lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
		lv_obj_set_style_border_color(panel, lv_color_hex(0x222222), LV_PART_MAIN);
		
		lv_obj_set_size(panel, 465, 35);
		lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, y);
		ui_item->panel = panel;
		obj = ui_helpers_create_label(panel, "", &lv_font_montserrat_14);
		lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 0);
		ui_item->title = obj;
		obj = ui_helpers_create_label(panel, "", &lv_font_montserrat_14);
		lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -20, 0);
		ui_item->value = obj;
		y += step;
	}
}
	