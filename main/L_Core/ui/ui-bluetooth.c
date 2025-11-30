#include "main.h"
#include "K_Core/K_Core.h"
#include "L_Core/bluetooth/ble.h"
#include "L_Core/storage/nvs.h"
#include "ui-bluetooth.h"
#include "L_Core/ui/ui-simple.h"

lv_obj_t* ui_ble_screen;
lv_obj_t* ui_ble_btn_scan;
lv_obj_t* ui_ble_spinner_scan;
lv_obj_t* ui_ble_device_list;
lv_obj_t* ui_ble_device_detail;
lv_obj_t* ui_ble_pair_button;
lv_obj_t* ui_ble_device_name;
lv_obj_t* ui_ble_device_address;
lv_obj_t* ui_ble_send;
lv_obj_t* ui_ble_receive;
lv_obj_t* ui_ble_total_received;
lv_obj_t* ui_ble_server_button;
lv_obj_t* ui_ble_client_button;
lv_obj_t* ui_ble_server_panel;
lv_obj_t* ui_ble_client_panel;
lv_obj_t* ui_ble_server_log_text;
lv_obj_t* ui_ble_server_sent_status;
lv_obj_t* ui_ble_server_receive_status;
lv_obj_t* ui_ble_server_name;

lv_obj_t* ui_ble_msg;
lv_obj_t* ui_ble_address_spin;
lv_obj_t* ui_ble_address_name;

lv_obj_t* ui_ble_server_xmit_enabled;
lv_obj_t* ui_ble_server_receive_enabled;
lv_obj_t* lbl_tx_indicator;
lv_obj_t* lbl_tx_num;
lv_obj_t* btn_show_tx;
lv_obj_t* btn_show_rx;
lv_obj_t* btn_clear;
lv_obj_t* lbl_rx_indicator;
lv_obj_t* lbl_rx_num;
lv_obj_t* btn_hex_display;
lv_obj_t* log_panel;
uint32_t log_head = 0;
uint32_t log_tail = 0;
char temp_string1[1024] = { 0 };
char temp_string2[256] = { 0 };

BleRemoteDevice* selected_device = NULL;
ble_server_status_t  prev_ble_server_status = BLE_SERVER_LISTENING;
// screen: 0: server screen
//			1: client screen

bool Show_rcv = false;
bool Show_xmt = false;
bool Hex_Format = false;
bool Sent_Blink = false;
bool Rcvd_Blink = false;

void ui_ble_switch_screen(uint8_t screen)
{
	if (screen == 0)
	{
		lv_obj_clear_flag(ui_ble_server_panel, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui_ble_client_panel, LV_OBJ_FLAG_HIDDEN);
		lv_obj_set_style_bg_color(ui_ble_server_button, lv_color_hex(UI_MENU_ACTIVE_ITEM_COLOR), LV_PART_MAIN);
		lv_obj_set_style_bg_color(ui_ble_client_button, lv_color_hex(UI_CHECK_NONACTIVE_COLOR), LV_PART_MAIN);
	}
	else
	{
		lv_obj_add_flag(ui_ble_server_panel, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui_ble_client_panel, LV_OBJ_FLAG_HIDDEN);
		lv_obj_set_style_bg_color(ui_ble_server_button, lv_color_hex(UI_CHECK_NONACTIVE_COLOR), LV_PART_MAIN);
		lv_obj_set_style_bg_color(ui_ble_client_button, lv_color_hex(UI_MENU_ACTIVE_ITEM_COLOR), LV_PART_MAIN);
	}
}

void ui_ble_show_setting_address()
{
	lv_obj_clear_flag(ui_ble_msg, LV_OBJ_FLAG_HIDDEN);
	lv_label_set_text(ui_ble_address_name, ble_get_name());
	lv_spinbox_set_value(ui_ble_address_spin, systemconfig.server_base_address);
}

void ui_ble_setting_button_event_cb(lv_event_t* e)
{
	int code = (int)e->user_data;
	if (!code)
	{
		systemconfig.server_base_address = lv_spinbox_get_value(ui_ble_address_spin);
		save_configuration();
		ble_update_base_address();
		lv_label_set_text(ui_ble_server_name, ble_get_name());
	}
	lv_obj_add_flag(ui_ble_msg, LV_OBJ_FLAG_HIDDEN);
}

void ui_ble_button_callback(lv_event_t* e) {
	int code = (int)lv_event_get_user_data(e);
	switch (code)
	{
	case UI_SIMPLE_BTN_XMIT:
		systemconfig.bluetooth.xmit_enabled = !systemconfig.bluetooth.xmit_enabled;
		Show_xmt = systemconfig.bluetooth.xmit_enabled;
		ui_change_button_color(btn_show_tx, 
			systemconfig.bluetooth.xmit_enabled == 0? 0x696969: 0x04fa05,
			0x0
		);
		break;
	case UI_SIMPLE_BTN_RCV:
		systemconfig.bluetooth.recv_enabled = !systemconfig.bluetooth.recv_enabled;
		Show_rcv = systemconfig.bluetooth.recv_enabled;
		ui_change_button_color(btn_show_rx, 
			systemconfig.bluetooth.recv_enabled == 0? 0x696969: 0x04fa05,
			0x0
		);
		break;
	case UI_SIMPLE_BTN_HEX:
		//lv_obj_add_flag(ui_simple_func_menu, LV_OBJ_FLAG_HIDDEN);
		Hex_Format = !Hex_Format;
		ui_change_button_color(btn_hex_display, 
			!Hex_Format ? 0x696969: 0x04fa05,
			0x0
			);
		break;
	case UI_SIMPLE_BTN_CLEAR:
		ClearLog();
		break;
	default:
		break;
	}
}


void ui_ble_spinbox_event_cb(lv_event_t* e)
{
	int code = (int)e->user_data;
	if (code)
	{
		lv_spinbox_increment(ui_ble_address_spin);
	}
	else 
		lv_spinbox_decrement(ui_ble_address_spin);
	
	strcpy(ui_temp_string, ble_get_name());
	int val = lv_spinbox_get_value(ui_ble_address_spin);
	char sz[4] = { 0 };
	
	sprintf(sz, "%03d",  val);
	strncpy(ui_temp_string + 9, sz, 3);
	lv_label_set_text(ui_ble_address_name, ui_temp_string);
}
void ui_ble_create_window_set_address()
{
	lv_obj_t *msgbox = ui_create_panel(ui_ble_screen, 0x444444, false);
	lv_obj_set_size(msgbox, 290, 210); // Resize the message box
	lv_obj_center(msgbox);
	
	uint16_t x = 10, y = 10, sh = 40, gap = 5;
	lv_obj_t* obj = ui_create_label(msgbox, "Please choose the number of the server base address.", &lv_font_montserrat_16);
	lv_obj_set_width(obj, 250);
	lv_obj_set_pos(obj, x+10, y); // Resize the message box
	
	y += sh*1.2 + gap;
	
	obj = ui_create_label(msgbox, ble_get_name(), &lv_font_montserrat_20);
	ui_ble_address_name = obj;
	lv_obj_set_pos(obj, 40, y);
	
	y += 40;
	x += 40;
	obj = lv_btn_create(msgbox);
	lv_obj_set_size(obj, sh, sh);
	lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_MINUS, 0);
	lv_obj_add_event_cb(obj, ui_ble_spinbox_event_cb, LV_EVENT_CLICKED, (void*)0);

	lv_obj_set_pos(obj, x, y);
	
	x += sh + gap;
	obj = lv_spinbox_create(msgbox);
	lv_spinbox_set_range(obj, 0, 255);
	lv_spinbox_set_digit_format(obj, 3, 0);
	lv_obj_set_style_pad_all(obj, 5, LV_PART_MAIN);
	lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_size(obj, 100, 40);
	lv_obj_set_pos(obj, x, y );
	ui_ble_address_spin = obj;
	x += 100 + gap;
	obj = lv_btn_create(msgbox);
	lv_obj_set_size(obj, sh, sh);
	lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_PLUS, 0);
	lv_obj_set_pos(obj, x, y);
	lv_obj_add_event_cb(obj, ui_ble_spinbox_event_cb, LV_EVENT_CLICKED, (void*)1);
	x = 100; y += sh + gap* 3;
	obj = ui_create_button(msgbox, "APPLY", 80, 40, 3, &lv_font_montserrat_14, ui_ble_setting_button_event_cb, (void*)0);
	lv_obj_set_pos(obj, x, y);
	x += 80+gap;
	obj = ui_create_button(msgbox, "CANCEL", 80, 40, 3, &lv_font_montserrat_14, ui_ble_setting_button_event_cb, (void*)1);
	lv_obj_set_pos(obj, x, y);
	ui_ble_msg = msgbox;
	
	
	
	lv_obj_add_flag(ui_ble_msg, LV_OBJ_FLAG_HIDDEN);
}

void ui_ble_server_address_event_cb(lv_event_t* e)
{
	
	ui_ble_show_setting_address();
}
void ui_ble_disconnect_event_cb(lv_event_t* e)
{
	ble_server_disconnect();
}

void ui_ble_server_updatename_event_cb(lv_event_t* e)
{
	requestBleNameTo407();
}

void ui_ble_scan_event_cb(lv_event_t* e) 
{
	//lv_obj_t * label = lv_obj_get_child(target, 0);
	switch (ble_scan_status)
	{
	case BLE_CLIENT_SCAN_READY: ble_scan_start();break;
	case BLE_CLIENT_SCANNING: ble_scan_stop();break;
	default: break;
	}
}

void ui_ble_timer_handler(lv_timer_t* timer)
{
	//if (ble_server_send_blink_count)ble_server_send_blink_count--; 
	if (!lv_obj_is_visible(ui_ble_screen)) return;
	if (ble_server_send_blink_count > 0)
	{
		lv_obj_set_style_text_color(lbl_tx_indicator, lv_color_hex(ble_server_send_blink_count % 2 ? UI_BUTTON_NORMAL_BG_COLOR : UI_BUTTON_NORMAL_FG_COLOR), LV_PART_MAIN);
		ble_server_send_blink_count--;
	}
	else 
	{
		lv_obj_set_style_text_color(lbl_tx_indicator, lv_color_hex(UI_BUTTON_DISABLE_BG_COLOR), LV_PART_MAIN);
	}
	if (ble_server_receive_blink_count > 0)
	{
		lv_obj_set_style_text_color(lbl_rx_indicator, lv_color_hex(ble_server_receive_blink_count % 2 ? UI_BUTTON_NORMAL_BG_COLOR : UI_BUTTON_NORMAL_FG_COLOR), LV_PART_MAIN);
		ble_server_receive_blink_count--;
	}
	else
	{
		lv_obj_set_style_text_color(lbl_rx_indicator, lv_color_hex(UI_BUTTON_DISABLE_BG_COLOR), LV_PART_MAIN);
	}

	if (ble_last_direction == 0 && systemconfig.bluetooth.xmit_enabled) 
	{
		// xmit karlchris   this is where we will plug in the display data
		add_log(ble_last_data_sent, UI_SEND_COLOR);
//		lv_textarea_set_text(ui_ble_server_log_text, ble_last_data);
//		lv_obj_set_style_text_color(ui_ble_server_log_text, lv_color_hex(UI_SEND_COLOR), LV_PART_MAIN);
	} else 
	if (ble_last_direction == 1 && systemconfig.bluetooth.recv_enabled)
	{
		// recv
		add_log(ble_last_data_rcvd, UI_RECEIVE_COLOR);
	}
}

void ui_ble_event_device_item_cb(lv_event_t* e) 
{
	//lv_obj_t * target = lv_event_get_target(e);
	BleRemoteDevice* device = (BleRemoteDevice*)lv_event_get_user_data(e);
	lv_label_set_text(ui_ble_device_name, device->device_name);
	lv_label_set_text(ui_ble_device_address, device->address);
	selected_device = device;
	ui_ble_set_device_status(selected_device);
}

void ui_ble_send_event_cb(lv_event_t* e)
{
	lv_obj_t* textobj = (lv_obj_t*)lv_event_get_user_data(e);
	char* text = (char*)lv_textarea_get_text(textobj);
	communication_add_string_to_ble_buffer(&bleDevice.TxBuffer, text);
}

void ui_ble_switch_event_cb(lv_event_t* e)
{
	uint8_t screen = (uint8_t)(int) e->user_data;	
	ui_ble_switch_screen(screen);
}

void ui_ble_pair_event_cb(lv_event_t* e)
{
	if(!selected_device) return;
	if(!selected_device->is_connected)
		ble_client_connect_device(selected_device);
	else
		ble_client_disconnect_device(selected_device);
}

void ui_ble_event_swap_cb(lv_event_t* e)
{
	ui_transform_screen(SCREEN_MEG);
}

void ui_ble_set_device_status(BleRemoteDevice* dev)
{
	lv_obj_t* item = ui_ble_get_item_by_device(dev);
	lv_obj_t* label = lv_obj_get_child(item, 0);
	if(dev->is_connected) {
		lv_label_set_text_fmt(label, "#66d800 " LV_SYMBOL_BLUETOOTH " %s #", dev->device_name);
		if(selected_device == dev) {
			lv_obj_set_style_bg_color(ui_ble_pair_button, lv_color_hex(0x3e8300), LV_PART_MAIN);
			label = lv_obj_get_child(ui_ble_pair_button, 0);
			lv_label_set_text(label, "UNPAIR DEVICE");
		}
	}else {
		lv_label_set_text_fmt(label, "#c4c4c4 " LV_SYMBOL_BLUETOOTH " %s #", dev->device_name);
		if(selected_device == dev) {
			lv_obj_set_style_bg_color(ui_ble_pair_button, lv_color_hex(0x4b4b4b), LV_PART_MAIN);
			label = lv_obj_get_child(ui_ble_pair_button, 0);
			lv_label_set_text(label, "PAIR DEVICE");
		}
	}
}

void ui_ble_screen_init()
{
	lv_obj_t* obj;
	ui_ble_screen = ui_create_screen();	
	ui_create_pct_title(ui_ble_screen, SCREEN_BLUETOOTH);
	
//	obj = ui_create_label(ui_ble_screen, (char*)"BLE", &lv_font_montserrat_14);	
//	lv_obj_set_pos(obj, 230, 5);
	
	int x = 20, y = 70;
	int button_large_width = 90;
	int button_h = 45;
	int gap = 5;
	
	y = 5;
	//server button
	obj = ui_create_button(ui_ble_screen, "  BLE\nSERVER", 90, button_h, 3, &lv_font_montserrat_14, ui_ble_switch_event_cb, (void*)0);	
	lv_obj_set_pos(obj, 105, y);
	ui_ble_server_button = obj;
	ui_change_button_color(obj, UI_MENU_ACTIVE_ITEM_COLOR, UI_BUTTON_NORMAL_FG_COLOR);
	//client button
	obj = ui_create_button(ui_ble_screen, "  BLE\nCLIENT", 90, button_h, 3, &lv_font_montserrat_14, ui_ble_switch_event_cb, (void*)1);	
	lv_obj_set_pos(obj, 385, y);
	ui_ble_client_button = obj;
	ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_NORMAL_FG_COLOR);
	// panel
	x = 5; y +=button_h + gap;
	obj = lv_obj_create(ui_ble_screen);
	lv_obj_set_size(obj, SCREEN_WIDTH-2, SCREEN_HEIGHT-65);
	lv_obj_set_pos(obj, 1, y); 
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	ui_ble_server_panel = obj;
	
	obj = lv_obj_create(ui_ble_screen);
	lv_obj_set_size(obj, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 65);
	lv_obj_set_pos(obj, 1, y); 
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	ui_ble_client_panel = obj;
	
	obj = ui_create_label(ui_ble_screen, ble_get_name(), &lv_font_montserrat_16);
	lv_obj_set_pos(obj, 210, 35); 
	ui_ble_server_name = obj;
	
	// server
	x = 10; y = 5;

	
	x = SCREEN_WIDTH - button_large_width - gap; y = 5;
	obj = ui_create_button(ui_ble_server_panel, "REQUEST", button_large_width, button_h, 2, &lv_font_montserrat_14, ui_ble_server_updatename_event_cb, NULL);
	lv_obj_set_pos(obj, x, y);
	y += button_h + gap;
	obj = ui_create_button(ui_ble_server_panel, "ADDRESS", button_large_width, button_h, 2, &lv_font_montserrat_14, ui_ble_server_address_event_cb, NULL);	
	lv_obj_set_pos(obj, SCREEN_WIDTH - button_large_width - gap, y);
	
	y = 5 + button_h + gap;
	x = 2;
	obj = ui_create_button(ui_ble_server_panel, "UN-PAIR", button_large_width, button_h, 3, &lv_font_montserrat_14, ui_ble_disconnect_event_cb, NULL);	
	lv_obj_set_pos(obj, SCREEN_WIDTH - button_large_width - gap, y+45);
	
	int button_w = 55;
	button_h = 45;
	gap = 5;
	//next is the clear button
	obj = ui_create_button(ui_ble_server_panel, "CLR", button_w, button_h, 2, &lv_font_montserrat_16, ui_ble_button_callback, (void*)UI_SIMPLE_BTN_CLEAR);
	//ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_DISABLE_FG_COLOR);
	ui_change_button_color(obj, UI_CHECK_NONACTIVE_COLOR, UI_BUTTON_NORMAL_FG_COLOR);
	//UI_CHECK_NONACTIVE_COLOR)

	lv_obj_set_pos(obj, x, 5);
	btn_clear = obj;
	
	const lv_font_t* font = &lv_font_montserrat_16;
	obj = ui_create_label(ui_ble_server_panel, LV_SYMBOL_UP, &lv_font_montserrat_20);
	lv_obj_set_style_text_color(obj, lv_color_hex(UI_BUTTON_DISABLE_FG_COLOR), LV_PART_MAIN);	
	lv_obj_set_pos(obj, x, y);
	lbl_tx_indicator = obj;
	
	obj = ui_create_label(ui_ble_server_panel, "0", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, x + 10, y + 5);
	lv_obj_set_width(obj, 50);
	lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, 0);	
	lbl_tx_num = obj;
	y += 25;

	
	obj = ui_create_button(ui_ble_server_panel, "XMT", button_w, button_h, 2, font, ui_ble_button_callback, (void*)UI_SIMPLE_BTN_XMIT);
	ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_DISABLE_FG_COLOR);
	lv_obj_set_pos(obj, x, y);
	btn_show_tx = obj;
	
	y += button_h + gap;
	obj = ui_create_button(ui_ble_server_panel, "RCV", button_w, button_h, 2, font, ui_ble_button_callback, (void*)UI_SIMPLE_BTN_RCV);
	ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_DISABLE_FG_COLOR);
	lv_obj_set_pos(obj, x, y);
	btn_show_rx = obj;
	
	y += button_h + gap;
	obj = ui_create_label(ui_ble_server_panel, LV_SYMBOL_DOWN, &lv_font_montserrat_20);
	lv_obj_set_style_text_color(obj, lv_color_hex(UI_BUTTON_DISABLE_FG_COLOR), LV_PART_MAIN);
	lv_obj_set_pos(obj, x, y);
	lbl_rx_indicator = obj;
	
	obj = ui_create_label(ui_ble_server_panel, "0", &lv_font_montserrat_12);
	lv_obj_set_pos(obj, x + 10, y);
	lv_obj_set_width(obj, 50);
	lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, 0);
	lbl_rx_num = obj;
	
	y += 25;
	obj = ui_create_button(ui_ble_server_panel, "HEX", button_w, button_h, 2, font, ui_ble_button_callback, (void*)UI_SIMPLE_BTN_HEX);
	//ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_DISABLE_FG_COLOR);
	ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_NORMAL_FG_COLOR);
	lv_obj_set_pos(obj, x, y);
	btn_hex_display = obj;
//
	obj = lv_obj_create(ui_ble_server_panel);
	lv_obj_set_size(obj, 320, SCREEN_HEIGHT - 70);
	lv_obj_set_pos(obj, button_w + gap * 2, 0); 
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	//lv_obj_set_style_bg_color(obj, lv_color_hex(UI_PANEL_BACGROUND_COLOR), LV_PART_MAIN);
	log_panel = obj;
	
	for (uint8_t i = 0; i < UI_LOG_MAX_LINE; i++)
	{
		obj = lv_label_create(log_panel);
		lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP); /*Automatically break long lines*/
		lv_obj_set_style_border_color(obj, lv_color_hex(0x550055), LV_PART_MAIN);
		lv_obj_set_style_text_font(obj, &mono_regualr_16, LV_PART_MAIN);
		lv_obj_set_width(obj, lv_pct(95)); 
		lv_obj_set_x(obj, 5); 
		lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
	}
	
//	y += button_h;
//	x = 5;
//	obj = lv_textarea_create(ui_ble_server_panel);
//	lv_obj_set_style_border_color(obj, lv_color_hex(UI_TEXTAREA_BORDER_COLOR), LV_PART_MAIN);
//	lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
//	 lv_textarea_set_one_line(obj, true);
//	 lv_obj_set_size(obj, 460, button_h);
//	 lv_obj_set_pos(obj, x, y);
//	 /* Make the server text area read-only: remove edit callback and disable click/focus
//		 so the user can't edit it via touch/keyboard. */
//	 lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
//	 ui_ble_server_log_text = obj;
	
	
	// client 
	x = 0; y = 0;
	obj = ui_create_button(ui_ble_client_panel, "SCAN", button_large_width, button_h, 3, &lv_font_montserrat_16, ui_ble_scan_event_cb, NULL);	
	lv_obj_set_pos(obj, x, y);	ui_ble_btn_scan = obj;
	
	obj = ui_create_button(ui_ble_client_panel, "PARING DEVICE", 240, button_h, 3, &lv_font_montserrat_16, ui_ble_pair_event_cb, NULL);
	ui_change_button_color(obj, UI_BUTTON_DISABLE_BG_COLOR, UI_BUTTON_NORMAL_FG_COLOR);
	lv_obj_set_pos(obj, 220, y); ui_ble_pair_button = obj;
	
	x = 120; 
	obj = lv_spinner_create(ui_ble_client_panel, 1000, 60);
	lv_obj_set_size(obj, 30, 30);	
	lv_obj_set_pos(obj, x, y); ui_ble_spinner_scan = obj;
	lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
	
	x = 0; y += button_h + gap;
	ui_ble_device_list = lv_obj_create(ui_ble_client_panel);
	lv_obj_set_size(ui_ble_device_list, 190, 150);
	lv_obj_set_pos(ui_ble_device_list, x, y); 
	lv_obj_set_style_pad_all(ui_ble_device_list, 2, LV_PART_MAIN);
	
	x = 210;
	ui_ble_device_detail = lv_obj_create(ui_ble_client_panel);
	lv_obj_clear_flag(ui_ble_device_detail, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_size(ui_ble_device_detail, 250, 200);
	lv_obj_set_pos(ui_ble_device_detail, x, y); 
	lv_obj_set_style_pad_all(ui_ble_device_detail, 2, LV_PART_MAIN);
	
	x = 10; y = 2;	
	obj = ui_create_label(ui_ble_device_detail, "NAME: ", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, x, y); 
	obj = ui_create_label(ui_ble_device_detail, "", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, 100, y); ui_ble_device_name = obj;
	
	
	x = 10; y += 25;	
	obj = ui_create_label(ui_ble_device_detail, "ADDRESS: ", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, x, y); 
	obj = ui_create_label(ui_ble_device_detail, "", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, 100, y); ui_ble_device_address = obj;
	
	y += 25;
	obj = lv_textarea_create(ui_ble_device_detail);
	lv_obj_set_style_border_color(obj, lv_color_hex(UI_TEXTAREA_BORDER_COLOR), LV_PART_MAIN);
	lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
	lv_textarea_set_one_line(obj, true);
	lv_obj_set_size(obj, 150, button_h);
	lv_obj_set_pos(obj, x, y);
	lv_obj_add_event_cb(obj, ui_event_edit_cb, LV_EVENT_ALL, NULL);	
	
	obj = ui_create_button(ui_ble_device_detail, "SEND", button_large_width, button_h, 3, &lv_font_montserrat_16, ui_ble_send_event_cb, obj);
	lv_obj_set_pos(obj, 160, y);
	
	y += button_h + gap;
	obj = ui_create_label(ui_ble_device_detail, "RECEIVED: ", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, x, y); 
	obj = ui_create_label(ui_ble_device_detail, "", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, 100, y);  ui_ble_receive = obj;
	
	y += 25;
	obj = ui_create_label(ui_ble_device_detail, "TOTAL: ", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, x, y);
	obj = ui_create_label(ui_ble_device_detail, "", &lv_font_montserrat_14);
	lv_obj_set_pos(obj, 100, y); ui_ble_total_received = obj;
	ui_ble_switch_screen(0);
	
	ui_change_button_color(btn_show_rx, 
			systemconfig.bluetooth.xmit_enabled == 0? 0x696969: 0x04fa05,
			0x0
		);

	ui_change_button_color(btn_show_tx, 
			systemconfig.bluetooth.recv_enabled == 0? 0x696969: 0x04fa05,
			0x0
		);
	
	ui_ble_create_window_set_address();
	lv_timer_create(ui_ble_timer_handler, 500, NULL);
}

void ui_ble_changed_ble_status(uint8_t status)
{
	if(!ui_initialized) return; // Do nothing util UI is initialized.
	lv_obj_t * label = lv_obj_get_child(ui_ble_btn_scan, 0);
	if (status == BLE_CLIENT_SCANNING)
	{
		lv_obj_clean(ui_ble_device_list);
		selected_device = NULL;
		lv_obj_clear_flag(ui_ble_spinner_scan, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text(label, "STOP");
	}
	else
	{
		lv_obj_add_flag(ui_ble_spinner_scan, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text(label, "SCAN");
	}
}

void ui_ble_add_device(void* dev) {
	if(!ui_initialized) return; // Do nothing util UI is initialized.
	BleRemoteDevice* device = (BleRemoteDevice*)dev;
	lv_obj_t* obj = lv_obj_create(ui_ble_device_list);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
	lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(obj, 2, LV_PART_MAIN);
	lv_obj_set_size(obj, LV_PCT(100), 45);
	lv_obj_set_pos(obj, 5, (45 + 5) * device->id);
	
	lv_obj_t* label = lv_label_create(obj);
	lv_label_set_recolor(label, true);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_label_set_text_fmt(label, "#c4c4c4 " LV_SYMBOL_BLUETOOTH " %s #", device->device_name);
	lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
	obj->user_data = (void*)device;
	lv_obj_add_event_cb(obj, ui_ble_event_device_item_cb, LV_EVENT_CLICKED, device);
}

lv_obj_t* ui_ble_get_item_by_device(BleRemoteDevice* dev)
{
	for (uint8_t i = 0; i < lv_obj_get_child_cnt(ui_ble_device_list); i++) {
		lv_obj_t* child = lv_obj_get_child(ui_ble_device_list, i);
		if(child->user_data == dev) {
			return child;
		}
	}
	return NULL;
}

void ui_ble_refresh_devices()
{
	lv_obj_clean(ui_ble_device_list); //remove all device items
	for(uint8_t i = 0; i < ble_client_scaned_device_num; i ++) 
	{
		ui_ble_add_device(&ble_client_remote_device[i]);
	}
}

void ui_ble_set_received_data(BleRemoteDevice* dev)
{
	if(selected_device != dev) return;
	lv_label_set_text(ui_ble_receive, (const char*)dev->last_received_buffer);
	sprintf(ui_temp_string, "%d", (int)dev->total_received);
	lv_label_set_text(ui_ble_total_received, ui_temp_string);
}

void ui_ble_set_servername(char* name)
{
	if (ui_ble_server_name)  lv_label_set_text(ui_ble_server_name, name); 
}

void add_line(const char* log, uint32_t color, bool isHex) {
	uint16_t index = log_head % UI_LOG_MAX_LINE;
	memset(temp_string1, 0, 1024);
	memset(temp_string2, 0, 256);
	lv_obj_t* obj = lv_obj_get_child(log_panel, index);
	lv_obj_set_height(obj, LV_SIZE_CONTENT);
	lv_obj_set_style_text_color(obj, lv_color_hex(color), LV_PART_MAIN);
	if (isHex)
	{
		uint16_t numberofcharcterstoadd = 16;
		uint16_t len =  strlen(log);
		if (numberofcharcterstoadd > len) numberofcharcterstoadd = len;
		int count = 0;
		char* hexString = temp_string1;
		char* asciiString = temp_string2;
		
		while (count < len)
		{
			sprintf(hexString, "%02X ", log[count]); hexString += 3;
			
			if ((log[count] >= ' ') & (log[count] <= 127))
			{
				*asciiString = (char)(log[count]);
			}
			else
			{
				*asciiString = '.';
			}
			asciiString++;
			count++;
			if ((count & 7) == 0)//we just got to 8 characters so display them and go to next line
			{
				*hexString = ' '; hexString++;
				strcpy(hexString, temp_string2);
				hexString += strlen(temp_string2);
				temp_string2[0] = '\0';
				*hexString = '\n';
				hexString++; 
				asciiString = temp_string2;
			}
		}
		int fill = (3 * (8 - (count & 7))); //lets see how many characters we need
		for (count = 0; count < fill - 1; count++)
		{ 
			*hexString = '-';
			hexString++;
		}
		*hexString = ' '; hexString++;
		*hexString = ' '; hexString++;
		strcpy(hexString, temp_string2);
		lv_label_set_text(obj, temp_string1);	
	}
	else
	{
		lv_label_set_text(obj, log);
	}
	//lv_obj_set_style_bg_color(obj, lv_color_hex(0x550055), LV_PART_MAIN);
	
	log_head++;
	log_tail = log_head < UI_LOG_MAX_LINE ? 0 : log_head - UI_LOG_MAX_LINE;
	
	uint16_t gap = 2;
	uint16_t ypos = 5;
	uint16_t idx = 0;
	uint16_t height = 0;
	for (index = log_tail; index < log_head; index++)
	{
		idx = index % UI_LOG_MAX_LINE;
		obj = lv_obj_get_child(log_panel, idx);
		lv_obj_set_y(obj, ypos);
		lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
		height = lv_obj_get_height(obj);
		ypos += height + gap;
	}
	lv_obj_scroll_to_y(log_panel, ypos, LV_ANIM_OFF);
}

void add_char(const char code, uint32_t color)
{
	switch (code)
	{
	case PING_CHAR:
		add_line("Send ping 0x7", UI_SEND_COLOR, false);
		break;
	case PING_REPLY:
		add_line("Reply ping 0x6", UI_RECEIVE_COLOR, false);
		break;
	}	
}
void add_log(char* log, uint32_t color)
{
	if (!log) return;
	int len = strlen(log);
	//log[len] = 0;
	if (!lv_obj_is_visible(log_panel)) return;
//	if ((color == UI_RECEIVE_COLOR && ui_simple_is_rcv) || //incomming data
//		(color == UI_SEND_COLOR && ui_simple_is_xmt))
//	{
		add_line(log, color, Hex_Format);
//	}
}

void ClearLog()
{
	log_head = 0;
	log_tail = 0;
	lv_obj_t* obj;
	//lv_obj_add_flag(ui_simple_func_menu, LV_OBJ_FLAG_HIDDEN);
	int count = lv_obj_get_child_cnt(log_panel);
	for (uint8_t i = 0; i < count; i++)
	{
		obj = lv_obj_get_child(log_panel, i); 
		lv_label_set_text(obj, "");
		lv_obj_set_x(obj, 5); 
		lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
	}
}
