#pragma once
#include "lvgl.h"
extern lv_obj_t* ui_simple_screen;

#define SIMPLE_MAX_ROW_SIZE 40
enum
{	
	BTN_FUNCS,
	BTN_XMIT,
	BTN_RCV,
	BTN_HEX,
	BTN_BLE,
	BTN_PING,
	BTN_STARTLOG,
	BTN_STOPLOG,
	BTN_S2F21W_ON,
	BTN_QUERY,
	BTN_UPDATERECIPE,
	BTN_DOWNLOADRECIPE,
	BTN_STARTPROCESS,
	BTN_STOPPROCESS,
	BTN_CANCELPROCESS,
	BTN_IDENTIFYGENERATOR,
	BTN_CLEAR,
	BTN_TEMP_ONOFF,
	BTN_TEMP_DECREASE,
	BTN_TEMP_INCREASE,
	
	UI_SIMPLE_CMD_PING = 50,
	UI_SIMPLE_CMD_START_LOGGING,
	UI_SIMPLE_CMD_STOP_LOGGING,
	UI_SIMPLE_CMD_QUERY,
	UI_SIMPLE_CMD_UPDATE_RECIPE,
	UI_SIMPLE_CMD_DOWNLOAD_RECIPE,
	UI_SIMPLE_CMD_START_PROCESS,
	UI_SIMPLE_CMD_CANCLE_PROCESS,
	UI_SIMPLE_CMD_IDENTIFY,
};

typedef struct
{
	lv_obj_t	*btn_rx, *btn_tx, *btn_hex, * btn_ble;
	lv_obj_t	*rx_indicator, *tx_indicator;
	lv_obj_t	*rx_num, *tx_num;
} UI_SIMPLE_OBJ;

extern bool ui_simple_is_rcv;
extern bool ui_simple_is_xmt;
extern bool ui_simple_is_ble;
void ui_simple_screen_init(void);
void ui_simple_add_log(char* log, uint32_t color);
void ui_simple_add_char(const char code, uint32_t color);
void ui_simple_add_line(const char* log, uint32_t color, bool isHex);
void ui_simple_clear();
void ui_simple_call_event_button(uint8_t code, bool direct);