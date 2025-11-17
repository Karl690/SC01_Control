#include "ui.h"
#include "ui-sdcard.h"
#include <dirent.h> 
#include <stdio.h>
#include <sys/stat.h>
#include <regex.h>

#include "../sd-card/sd-card.h"
#include "K_Core/execution/cmdprocessor.h"

UI_SDCARD ui_sdcard;
UI_SDCARD_FILEVIEW ui_sdcard_fileview;
lv_obj_t* ui_sdcard_screen;
lv_obj_t* selected_item;
int ui_sdcard_file_lines = 0;
bool ui_sdcard_update_flag = false;
char current_sdcard_path[MAXNAMLEN + 1] = SDCARD_MOUNT_POINT;

int ui_sdcard_spin_max = 0;
const char *get_file_extension(const char *filename) {
	const char *dot = strchr(filename, '.');
	if (!dot || *(dot + 1) == '\0') {
		return "";
	}
	return dot + 1;
}

int extract_index_with_regex(const char *filename) {
	regex_t regex;
	regmatch_t matches[2];
	int ret;
	char *pattern = "^.*[^0-9]([0-9]+)\\.[^.]+$";
    
	if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
		return -1;
	}

	ret = regexec(&regex, filename, 2, matches, 0);
	regfree(&regex);

	if (ret != 0 || matches[1].rm_so == -1) {
		return -1;
	}

	// ??? ?? ?? ??
	char num_str[32];
	int len = matches[1].rm_eo - matches[1].rm_so;
	strncpy(num_str, filename + matches[1].rm_so, len);
	num_str[len] = '\0';

	return atoi(num_str);
}

int extract_index_from_filename(const char *filename, char* prefix) {
	int prefix_len = strlen(prefix);
	char* surfix = (char*)(filename + prefix_len);
	char *dot = strrchr(surfix, '.');
	if (dot == NULL) {
		return -1;
	}
	size_t basename_len = dot - surfix;
    
	int digit_count = 0;
	int number = 0;
	int multiplier = 1;
    
	for (int i = basename_len - 1; i >= 0; i--) {
		if (isdigit(surfix[i])) {
			number += (surfix[i] - '0') * multiplier;
			multiplier *= 10;
			digit_count++;
		}
		else {
			break;
		}
	}

	return digit_count > 0 ? number : -1;
}

int find_next_available_index(const char *dir_path, char *prefix) {
	DIR *dir = opendir(dir_path);
	if (!dir) return 1;

	int max_index = 0;
	struct dirent *entry;
    
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG && strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
			int current_index = extract_index_from_filename(entry->d_name, prefix);
			if (current_index > max_index) {
				max_index = current_index;
			}
		}
	}
	closedir(dir);
    
	return max_index + 1;
}

void ui_sdcard_text_panel_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_LONG_PRESSED)
	{
		if (!lv_obj_is_visible(ui_sdcard_fileview.nav_panel)) 
			lv_obj_clear_flag(ui_sdcard_fileview.nav_panel, LV_OBJ_FLAG_HIDDEN);
	}
	else if (code == LV_EVENT_PRESSED)
	{
		if (lv_obj_is_visible(ui_sdcard_fileview.nav_panel)) 
			lv_obj_add_flag(ui_sdcard_fileview.nav_panel, LV_OBJ_FLAG_HIDDEN);
	}
}
void ui_sdcard_read_file_by_line(int line)
{
	char* filepath = lv_label_get_text(ui_sdcard_fileview.file_path);
	FILE *file = fopen(filepath, "r");
	if (!file) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Error opening file", 1000);
		return;
	}
	
	char buffer[256];
	int index = 0;
	int cnt = 0;
	
	for (int i = 0; i < UI_SDCARD_MAX_DISPLAY_LINES; i++)
		lv_label_set_text(ui_sdcard_fileview.view_text_item[i], "");
	while (fgets(buffer, sizeof(buffer), file)) {
		if (index >= line && index < line + UI_SDCARD_MAX_DISPLAY_LINES)
		{
			// lv_textarea_add_text(ui_sdcard.view_text_area, buffer);
			// lv_textarea_add_char(ui_sdcard.view_text_area, '\n');
			sprintf(ui_temp_string, "%05d %s", index, buffer);
			lv_label_set_text(ui_sdcard_fileview.view_text_item[cnt], ui_temp_string);
			cnt++;
		}
		else if (index > line + UI_SDCARD_MAX_DISPLAY_LINES)
		{
			break;
		}
		index++;
	}

	if (ferror(file)) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Error reading file", 3000);
	}
	
	fclose(file);
}

void ui_sdcard_nav_panel_event_cb(lv_event_t* e)
{
	int data = (int)lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	switch (data)
	{
	case 10: // navigate to first
		lv_spinbox_set_value(ui_sdcard_fileview.page, 0);
		ui_sdcard_read_file_by_line((int)lv_spinbox_get_value(ui_sdcard_fileview.page));
		break;
	case 11: // navigate to prev
		if (code == LV_EVENT_LONG_PRESSED_REPEAT || code == LV_EVENT_PRESSED)
		{
			lv_spinbox_decrement(ui_sdcard_fileview.page);
		}
		else if (code == LV_EVENT_RELEASED)
		{
			ui_sdcard_read_file_by_line((int)lv_spinbox_get_value(ui_sdcard_fileview.page));
		}
		break;
	case 12: // navigate to next
		if (code == LV_EVENT_LONG_PRESSED_REPEAT || code == LV_EVENT_PRESSED)
		{
			lv_spinbox_increment(ui_sdcard_fileview.page);
		}
		else if (code == LV_EVENT_RELEASED)
		{
			ui_sdcard_read_file_by_line((int)lv_spinbox_get_value(ui_sdcard_fileview.page));
		}
		break;
	case 13: // navigate to last
		lv_spinbox_set_value(ui_sdcard_fileview.page, ui_sdcard_spin_max);
		ui_sdcard_read_file_by_line(ui_sdcard_spin_max);
		break;
	}
}

void event_open_file_cb(lv_event_t* e) 
{
	lv_obj_t* item = lv_event_get_target(e);
	if (item == selected_item)
	{
		return;
	}
	if (selected_item != NULL)	lv_obj_set_style_bg_color(selected_item, lv_color_hex(UI_LIST_ITEM_BG_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_color(item, lv_color_hex(UI_LIST_ITEM_ACTIVE_COLOR), LV_PART_MAIN);
	selected_item = item;
}
void event_goto_directory_cb(lv_event_t* e)
{
	char* dir_name = (char*)lv_event_get_user_data(e);
	//(char*)obj->user_data;
	if (!dir_name) return;
	char new_path[MAXNAMLEN + MAXNAMLEN] = { 0 };
	if (!strcmp(dir_name, UP_DIRECTORY))
	{	
		int16_t i = 0;
		for (i = strlen(current_sdcard_path); i >= 0; i--)
		{
			if (current_sdcard_path[i] == '/') break;			
		}
		if (i > 0) {
			strncpy(new_path, current_sdcard_path, i);
			ui_sdcard_load_directory(new_path);
		}
	}
	else
	{
		sprintf(new_path, "%s/%s", current_sdcard_path, dir_name);
		ui_sdcard_load_directory(new_path);
	}
	selected_item = NULL;
}

void remove_top_append_bottom(lv_obj_t * ta) {
	// Get current text
	const char * text = lv_textarea_get_text(ta);
    
	// Find the first newline character
	const char * first_newline = strchr(text, '\n');
    
	if (first_newline) {
		// If there is a newline, skip the first line (text after first newline)
		const char * remaining_text = first_newline + 1;
        
		// Set the textarea content to everything after the first line
		lv_textarea_set_text(ta, remaining_text);
	}
	else {
		// If no newline, just clear the textarea (only one line)
		lv_textarea_set_text(ta, "");
	}
    
	// Append a new line at the bottom
	lv_textarea_add_char(ta, '\n');
}
void ui_sdcard_open_file(char* filepath)
{
	FILE *file = fopen(filepath, "r");
	if (!file) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Error opening file", 3000);
		return;
	}
	char buffer[256];
	int index = 0;
	while (fgets(buffer, sizeof(buffer), file)) {
		index++;
	}

	if (ferror(file)) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Error reading file", 3000);
		return;
	}
	
	fclose(file);
	ui_sdcard_file_lines = index;
	if (index - UI_SDCARD_MAX_DISPLAY_LINES < 0) index = 0;
	else index -= UI_SDCARD_MAX_DISPLAY_LINES;
	ui_sdcard_spin_max = index;
	lv_label_set_text(ui_sdcard_fileview.file_path, filepath);
	lv_spinbox_set_range(ui_sdcard_fileview.page, 0, ui_sdcard_spin_max);
	lv_spinbox_set_value(ui_sdcard_fileview.page, 0);
	ui_sdcard_read_file_by_line(0);
	lv_obj_clear_flag(ui_sdcard_fileview.panel, LV_OBJ_FLAG_HIDDEN);
}

char* ui_sdcard_get_opened_filepath()
{
	if (!lv_obj_is_visible(ui_sdcard_fileview.panel)) return NULL;
	return lv_label_get_text(ui_sdcard_fileview.file_path);
}

void ui_sdcard_delete_file(char* filepath)
{
	if (unlink(filepath) != 0) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Delete file falied.", 3000);
		return;
	}
	ui_sdcard_load_directory(current_sdcard_path);
	ui_show_messagebox(MESSAGEBOX_INFO, "Delete file succcessful.", 2000);
}

char prefix[256] = { 0 };
char extension[4] = { 0 };
void ui_sdcard_archive_file(char* filepath)
{
	char* filename = (char*)selected_item->user_data;
	int len = strlen(filename);
	
	strncpy(extension, filename + len - 3, 3);
	strncpy(prefix, filename, len - 4);
	
	int index = find_next_available_index(current_sdcard_path, prefix);
	sprintf(temp_string, "%s/%s_%03d.%s", current_sdcard_path, prefix, index, extension);
	int result = rename(filepath, temp_string);
	if (result != 0) {
		ui_show_messagebox(MESSAGEBOX_ERROR, "Archive file falied.", 3000);
		return;
	}
	FILE* fp = fopen(filepath, "w");
	if (fp) fclose(fp);
	ui_sdcard_load_directory(current_sdcard_path);
	ui_show_messagebox(MESSAGEBOX_INFO, "Archive file succcessful.", 2000);
}
void ui_sdcard_event_button_cb(lv_event_t* e)
{
	if (selected_item == NULL) return;
	
	int data = (int)lv_event_get_user_data(e);
	sprintf(ui_temp_string, "%s/%s", current_sdcard_path, selected_item->user_data);
	
	switch (data)
	{
	case 1: // open file 
		{
			
			const char* extension = get_file_extension(ui_temp_string);
			if (!strcmp(extension, "txt") || !strcmp(extension, "csv"))
			{
				ui_sdcard_open_file(ui_temp_string);	
			}
			else
			{
				ui_show_messagebox(MESSAGEBOX_WARNING, "Not supported file format.", 3000);
			}
		}
		break;
	
	case 2: // delete file
		ui_sdcard_delete_file(ui_temp_string);
		break;
	case 3:
		ui_sdcard_archive_file(ui_temp_string);
		break;
	case 10: // close file view panel
		lv_obj_add_flag(ui_sdcard_fileview.panel, LV_OBJ_FLAG_HIDDEN);
		break;
	}
}

void ui_sdcard_update_timer(lv_timer_t* timer) {
	if (!lv_obj_is_visible(ui_sdcard_screen)) return;
	if (!systemconfig.sdcard.status)
	{	
		lv_obj_clear_flag(ui_sdcard.status_msg, LV_OBJ_FLAG_HIDDEN);	
	}
	else
	{
		lv_obj_add_flag(ui_sdcard.status_msg, LV_OBJ_FLAG_HIDDEN);	
	}
	
	if (!cmd_sdcard_write_flag) {
		return;
	}
	cmd_sdcard_write_flag = false;
	if (!lv_obj_is_visible(ui_sdcard_fileview.panel)) return;
	char* filepath = lv_label_get_text(ui_sdcard_fileview.file_path);
	if (!strstr(filepath, "meg407.txt")) return;
	ui_sdcard_file_lines++;
	int index = ui_sdcard_file_lines;
	if (index - UI_SDCARD_MAX_DISPLAY_LINES < 0) index = 0;
	else index -= UI_SDCARD_MAX_DISPLAY_LINES;
	ui_sdcard_spin_max = index;
	
	lv_spinbox_set_range(ui_sdcard_fileview.page, 0, ui_sdcard_spin_max);
	ui_sdcard_read_file_by_line(ui_sdcard_spin_max);
}


void ui_sdcard_screen_init(void)
{
	ui_sdcard_screen = ui_create_screen();	
	ui_create_pct_title(ui_sdcard_screen, SCREEN_SDCARD);
	
	lv_obj_t* obj = lv_label_create(ui_sdcard_screen);	
	lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);	
	lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_label_set_text(obj, "SD CARD");
	lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 100, 5);
	
	
	obj = lv_label_create(ui_sdcard_screen);	
	lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_label_set_text(obj, "PATH: /sd-card");
	lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 10, 60);
	ui_sdcard.dir_path = obj;
	
	obj = lv_obj_create(ui_sdcard_screen);
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	lv_obj_set_size(obj, 210, 235);
	lv_obj_set_pos(obj, 3, 75);
	ui_sdcard.panel_dir = obj;
	
	obj = lv_obj_create(ui_sdcard_screen);
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	lv_obj_set_size(obj, 261, 235);
	lv_obj_set_pos(obj, 216, 75);
	ui_sdcard.panel_file = obj;
	
	int button_w = 80;
	int button_h = 50;
	int start_x = 230;
	obj = ui_create_button(ui_sdcard_screen, "Open", button_w, button_h, 2, &lv_font_montserrat_16, ui_sdcard_event_button_cb, (void*)1);
	lv_obj_set_pos(obj, start_x, 5);
	obj = ui_create_button(ui_sdcard_screen, "Delete", button_w, button_h, 2, &lv_font_montserrat_16, ui_sdcard_event_button_cb, (void*)2);
	lv_obj_set_pos(obj, start_x + button_w + 5, 5);
	obj = ui_create_button(ui_sdcard_screen, "Archive", button_w, button_h, 2, &lv_font_montserrat_16, ui_sdcard_event_button_cb, (void*)3);
	lv_obj_set_pos(obj, start_x + (button_w  + 5)*2, 5);
	
	ui_sdcard_fileview_init();
}

void ui_sdcard_fileview_init()
{
	lv_obj_t* obj = lv_obj_create(ui_sdcard_screen);
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	lv_obj_set_size(obj, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
	ui_sdcard_fileview.panel = obj;
	
	obj = ui_create_button(ui_sdcard_fileview.panel, LV_SYMBOL_CLOSE, 35, 35, 17, &lv_font_montserrat_16, ui_sdcard_event_button_cb, (void*)10);
	lv_obj_set_style_bg_color(obj, lv_color_hex(UI_LIST_ITEM_ACTIVE_COLOR), LV_PART_MAIN);
	lv_obj_set_pos(obj, 435, 2);
	obj = ui_create_label(ui_sdcard_fileview.panel, "/sd-card/", &lv_font_montserrat_14);
	ui_sdcard_fileview.file_path = obj;
	lv_obj_set_pos(obj, 2, 5);
	
	ui_sdcard_fileview.view_text_panel = lv_obj_create(ui_sdcard_fileview.panel);
	lv_obj_set_style_pad_all(ui_sdcard_fileview.view_text_panel, 1, LV_PART_MAIN);
	lv_obj_set_scroll_dir(ui_sdcard_fileview.view_text_panel, LV_DIR_HOR);
	lv_obj_set_size(ui_sdcard_fileview.view_text_panel, 470, 275);
	lv_obj_set_pos(ui_sdcard_fileview.view_text_panel, 2, 40);
	lv_obj_add_flag(ui_sdcard_fileview.view_text_panel, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ui_sdcard_fileview.view_text_panel, ui_sdcard_text_panel_event_cb, LV_EVENT_ALL, NULL);
	for (int i = 0; i < UI_SDCARD_MAX_DISPLAY_LINES; i++)
	{
		obj = lv_label_create(ui_sdcard_fileview.view_text_panel);
		lv_obj_set_style_text_font(obj, &mono_regualr_20, LV_PART_MAIN);
		ui_sdcard_fileview.view_text_item[i] = obj;
		sprintf(ui_temp_string, "%05d ", i + 1);
		lv_label_set_text(obj, ui_temp_string);
		lv_obj_set_pos(obj, 0, i * 26);
	}
	
	ui_sdcard_fileview.nav_panel = ui_create_panel(ui_sdcard_fileview.panel, 0xDDDDDD,  false);
	lv_obj_set_style_bg_opa(ui_sdcard_fileview.nav_panel, 128, LV_PART_MAIN);
	lv_obj_set_align(ui_sdcard_fileview.nav_panel, LV_ALIGN_CENTER);
	int x = 5;
	obj = ui_create_button(ui_sdcard_fileview.nav_panel, LV_SYMBOL_PREV, 40, 40, 3, &lv_font_montserrat_20, ui_sdcard_nav_panel_event_cb, (void*)10);
	lv_obj_set_pos(obj, x, 5);
	x += 45;
	obj = ui_create_button(ui_sdcard_fileview.nav_panel, LV_SYMBOL_LEFT, 40, 40, 3, &lv_font_montserrat_20, NULL, (void*)11);
	lv_obj_add_event_cb(obj, ui_sdcard_nav_panel_event_cb, LV_EVENT_ALL, (void*)11);
	lv_obj_set_pos(obj, x, 5);
	x += 45;
	obj = lv_spinbox_create(ui_sdcard_fileview.nav_panel);
	lv_spinbox_set_range(obj, 0, 10000);
	lv_obj_set_style_pad_all(obj, 2, LV_PART_MAIN);
	lv_obj_set_style_text_font(obj, &mono_bold_24, LV_PART_MAIN);
	lv_spinbox_set_value(obj, 1888);
	lv_obj_set_width(obj, 70);
	lv_obj_set_pos(obj, x, 8);
	ui_sdcard_fileview.page = obj;
	x += 75;
	obj = ui_create_button(ui_sdcard_fileview.nav_panel, LV_SYMBOL_RIGHT, 40, 40, 3, &lv_font_montserrat_20, NULL, (void*)12);
	lv_obj_add_event_cb(obj, ui_sdcard_nav_panel_event_cb, LV_EVENT_ALL, (void*)12);
	lv_obj_set_pos(obj, x, 5);
	x += 45;
	obj = ui_create_button(ui_sdcard_fileview.nav_panel, LV_SYMBOL_NEXT, 40, 40, 3, &lv_font_montserrat_20, ui_sdcard_nav_panel_event_cb, (void*)13);
	lv_obj_set_pos(obj, x, 5);
	x += 45;
	lv_obj_set_size(ui_sdcard_fileview.nav_panel, x, 50);
	lv_obj_add_flag(ui_sdcard_fileview.nav_panel, LV_OBJ_FLAG_HIDDEN);
	
	ui_sdcard.status_msg = ui_create_panel(ui_sdcard_screen, ERROR_COLOR, false);
	lv_obj_set_size(ui_sdcard.status_msg, 300, 50);
	lv_obj_align(ui_sdcard.status_msg, LV_ALIGN_CENTER, 0, 0);
	obj = ui_create_label(ui_sdcard.status_msg, "NO SD CARD DETECTED", &lv_font_montserrat_20);
	lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_flag(ui_sdcard.status_msg, LV_OBJ_FLAG_HIDDEN);
	lv_timer_create(ui_sdcard_update_timer, 1000, NULL);
}

void ui_sdcard_add_directory(const char* path, uint16_t index)
{
	lv_obj_t* obj = lv_obj_create(ui_sdcard.panel_dir);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
	lv_obj_set_style_bg_color(obj, lv_color_hex(UI_LIST_ITEM_BG_COLOR), LV_PART_MAIN);
	lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN);
	lv_obj_set_size(obj, LV_PCT(100), 40);
	lv_obj_set_pos(obj, 5, (40 + 5) * index);
	
	lv_obj_t* label = lv_label_create(obj);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_label_set_text(label, path);
	lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
	obj->user_data = lv_label_get_text(label);
	lv_obj_add_event_cb(obj, event_goto_directory_cb, LV_EVENT_CLICKED, (void*)obj->user_data);
}


void ui_sdcard_add_file(const char* path, uint16_t index)
{
	lv_obj_t* obj = lv_obj_create(ui_sdcard.panel_file);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_style_bg_color(obj, lv_color_hex(UI_LIST_ITEM_BG_COLOR), LV_PART_MAIN);
	lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN);
	lv_obj_set_size(obj, LV_PCT(100), 40);
	lv_obj_set_pos(obj, 5, (40 + 5) * index);
	
	lv_obj_t* label = lv_label_create(obj);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_label_set_text_fmt(label, path);
	lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
	obj->user_data = lv_label_get_text(label);
	lv_obj_add_event_cb(obj, event_open_file_cb, LV_EVENT_CLICKED, (void*)obj->user_data);
}

void ui_sdcard_load_directory(const char* path)
{
	selected_item = NULL;
	strcpy(current_sdcard_path, path);
	lv_label_set_text_fmt(ui_sdcard.dir_path, "PATH: %s", current_sdcard_path);
	lv_obj_clean(ui_sdcard.panel_dir);
	lv_obj_clean(ui_sdcard.panel_file);
			
	DIR * d = opendir(path); // open the path
	if (d == NULL) return; // if was not able, return
	struct dirent * dir; // for the directory entries
	uint16_t cnt_folder = 0;
	uint16_t cnt_file = 0;
	ui_sdcard_add_directory("..", cnt_folder);
	cnt_folder++;
	while ((dir = readdir(d)) != NULL) // if we were able to read somehting from the directory
	{
		if (dir->d_type != DT_DIR) // if the type is not directory just print it with blue color
		{
			ui_sdcard_add_file(dir->d_name, cnt_file);
			cnt_file++;
		}
		else if(dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) // if it is a directory 
		{	
			ui_sdcard_add_directory(dir->d_name, cnt_folder);
			cnt_folder++;
		}
	}
	closedir(d); // finally close the directory
}

