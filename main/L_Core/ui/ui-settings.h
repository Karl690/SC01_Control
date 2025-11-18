#pragma once


typedef enum
{
	SETTINGS_SUBMENU_MODE,
	SETTINGS_SUBMENU_BLUETOOTH,
	SETTINGS_SUBMENU_WIFI,
	SETTINGS_SUBMENU_OPC,
	SETTINGS_SUBMENU_SERIAL1,
	SETTINGS_SUBMENU_SERIAL2,
	SETTINGS_SUBMENU_SECS,
	SETTINGS_SUBMENU_SDCARD,
	SETTINGS_SUBMENU_SYSTEM,
}SETTINGS_SUBMENU_TYPE;
typedef struct
{
	lv_obj_t* option;
} UI_SETTINGS_MODE;

typedef struct
{
	lv_obj_t* ssid;
	lv_obj_t* password;
	lv_obj_t* status;
	lv_obj_t* autoconnect;
	lv_obj_t* ip;
	lv_obj_t* subnet;
} UI_SETTINGS_WIFI;

typedef struct
{
	lv_obj_t* status;
	lv_obj_t* autostart;
	lv_obj_t* screen_control;
} UI_SETTINGS_BLUETOOTH;

typedef struct
{
	lv_obj_t* status;
	lv_obj_t* autostart;
	lv_obj_t* name;
	lv_obj_t* password;	
} UI_SETTINGS_OPC;

typedef struct
{
	lv_obj_t* status;
	lv_obj_t* automount;	
} UI_SETTINGS_SDCARD;

typedef struct
{
	lv_obj_t* ui_rx_pin;
	lv_obj_t* ui_tx_pin;
	lv_obj_t* ui_baud;
	lv_obj_t* ui_mode;
} UI_SETTINGS_SERIAL;

typedef struct
{
	lv_obj_t* ui_timer_reload1;
	lv_obj_t* ui_timer_reload2;
	lv_obj_t* ui_timer_retry;
} UI_SETTINGS_SECS;

typedef struct
{
	UI_SETTINGS_MODE ui_mode;
	UI_SETTINGS_WIFI ui_wifi;
	UI_SETTINGS_BLUETOOTH ui_bluetooth;
	UI_SETTINGS_OPC ui_opc;
	UI_SETTINGS_SDCARD ui_sdcard;
	UI_SETTINGS_SERIAL ui_serial1;
	UI_SETTINGS_SERIAL ui_serial2;
	UI_SETTINGS_SECS ui_secs;
}UI_SETTINGS;

extern lv_obj_t* ui_settings_screen;
extern UI_SETTINGS ui_settings;

void ui_settings_screen_init();
void ui_settings_update_configuratiion();