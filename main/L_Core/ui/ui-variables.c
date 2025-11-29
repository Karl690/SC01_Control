#include "ui.h"
#include "ui-variables.h"
#include "K_Core/display/DisplayList.h"

#define UI_VARIABLES_ITEM_SIZE 8

lv_obj_t* ui_variable_screen;

lv_obj_t* Diag_line_1;
lv_obj_t* Diag_line_2;
lv_obj_t* Diag_line_3;
lv_obj_t* Diag_line_4;
lv_obj_t* Diag_line_5;
lv_obj_t* Diag_line_6;
lv_obj_t* Diag_line_7;
lv_obj_t* Diag_line_8;
lv_obj_t* Diag_line_9;
lv_obj_t* Diag_line_10;
lv_obj_t* Diag_line_11;

lv_obj_t* Diag_labels[11];
char diagDisplayBuffer[60];
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
	if (!lv_obj_is_visible(ui_variable_screen)) return;//dont process if not visible
	int len = 0;// strlen(value);
	char tempbuff[20];
	DisplayVariableInfo* activeDisplayVarInfo = LcdVarsTable; //default
	switch (DisplayIndex) 
	{
	case 0:
		activeDisplayVarInfo = LcdVarsTable;// Lcd1VarsTable;
		break;
	case 1:
		activeDisplayVarInfo = Lcd1VarsTable; //Lcd1VarsTable;
		break;
	case 2:
		activeDisplayVarInfo = LcdVarsTable; //Lcd2VarsTable;
		break;
	case 3:
		activeDisplayVarInfo = Lcd1VarsTable; //Lcd3VarsTable;
		break;
	}
	//set max lines to 11 for now
	for (int i = 0; i < 10; i ++) 
	{
		DisplayVariableInfo* info = &activeDisplayVarInfo[i];
		//UI_VARIABLES_ITEM	* item = &ui_variables_ui_items[i];
		if (!activeDisplayVarInfo[i].VariablePointer) 
		{
			lv_label_set_text(Diag_labels[i], ""); //clear out text if not valid
		} 
		else 
		{
			DisplayVariableInfo* info = &activeDisplayVarInfo[i];
			//item->data_item = info;
			//lv_label_set_text(item->title, info->Label);
			
//			sprintf(values, "0x%X", buf[k + i]);
//			strcat(row_string, values);
			int len = strlen(info->Label);
			
			memset(diagDisplayBuffer,(int) 0,60);//clear out 60 old characters
			
			strncpy(diagDisplayBuffer, info->Label, len); //load up the label

			switch (info->FuncType)
			{
			case FUNC_INT32:
				sprintf(ui_temp_string, "%d", *(int*)info->VariablePointer);
				len = strlen(ui_temp_string);
				strncat(diagDisplayBuffer, ui_temp_string, len);
				lv_label_set_text(Diag_labels[i], diagDisplayBuffer);
				break;
			case FUNC_INT16:
				sprintf(ui_temp_string, "%d", *(uint16_t*)info->VariablePointer);
				len = strlen(ui_temp_string);
				strncat(diagDisplayBuffer, ui_temp_string, len);
				lv_label_set_text(Diag_labels[i], diagDisplayBuffer);
				break;
			case FUNC_FLOAT:
				sprintf(ui_temp_string, "%.2f", *(float*)info->VariablePointer);
				len = strlen(ui_temp_string);
				strncat(diagDisplayBuffer, ui_temp_string, len);
				lv_label_set_text(Diag_labels[i], diagDisplayBuffer);
				break;
			case FUNC_BOOLEAN:
				lv_label_set_text_fmt(Diag_labels[i], "%s", *(bool*)info->VariablePointer == 1 ? "ON" : "OFF");
				break;
			default:
				lv_label_set_text(Diag_labels[i], (char*)info->VariablePointer);
				break;
			}
		}
	}
}

void ui_variables_screen_init(void)
{	
	ui_variable_screen = ui_create_screen();	
	ui_create_pct_title(ui_variable_screen, SCREEN_VARIABLES);
	
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

	int x = 5, y = 40, gap = 5;

	lv_obj_t* container = ui_helpers_create_panel(ui_variable_screen, 0x00000, false);
	lv_obj_set_size(container, 470, 256);//woking size of dispaly
	lv_obj_set_pos(container, 5, 60);//set start of panel just below title
	//instantiate the line instances
	
	obj = ui_create_label(ui_variable_screen, "heartbeat", &mono_regualr_20); lv_obj_set_pos(obj, x, y); 
	Diag_line_1 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_2 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "bat1", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_3 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "2", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_4 = obj;
	y += 22 ;
	obj = ui_create_label(ui_variable_screen, "3", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_5 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "4", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_6 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "5", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_7 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "6", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_8 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "7", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_9 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "8", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_10 = obj;
	y += 22;
	obj = ui_create_label(ui_variable_screen, "9", &mono_regualr_20); lv_obj_set_pos(obj, x, y);
	Diag_line_11 = obj;
//
	Diag_labels[0] = Diag_line_1;
	Diag_labels[1] = Diag_line_2;
	Diag_labels[2] = Diag_line_3;
	Diag_labels[3] = Diag_line_4;
	Diag_labels[4] = Diag_line_5;
	Diag_labels[5] = Diag_line_6;
	Diag_labels[6] = Diag_line_7;
	Diag_labels[7] = Diag_line_8;
	Diag_labels[8] = Diag_line_9;
	Diag_labels[9] = Diag_line_10;
	Diag_labels[10] = Diag_line_11;

}
	