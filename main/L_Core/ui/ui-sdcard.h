#pragma once
#include "ui.h"

#define UP_DIRECTORY ".."
#define UI_SDCARD_MAX_DISPLAY_LINES 10


typedef struct _tagUI_SDCARD_FILEVIEW
{
	lv_obj_t* panel;
	lv_obj_t* file_path;
	lv_obj_t* view_text_panel;
	lv_obj_t* view_text_item[UI_SDCARD_MAX_DISPLAY_LINES];
	lv_obj_t* nav_panel;
	lv_obj_t* page;
	lv_obj_t* first;
	lv_obj_t* prev;
	lv_obj_t* next;
	lv_obj_t* last;
} UI_SDCARD_FILEVIEW;

typedef struct _tagUI_SDCARD
{
	lv_obj_t* dir_path;
	lv_obj_t* panel_file;
	lv_obj_t* panel_dir;
	lv_obj_t* status_msg;
}UI_SDCARD;

extern char current_sdcard_path[];
extern bool ui_sdcard_update_flag;
extern lv_obj_t* ui_sdcard_screen;

void ui_sdcard_screen_init(void);
void ui_sdcard_load_directory(const char*);

void ui_sdcard_fileview_init();
char* ui_sdcard_get_opened_filepath();
void ui_sdcard_open_file(char* filepath);