#include "M_Codes.h"
#include "sequencer.h"
#include "L_Core/ui/ui-simple.h"
#include "K_Core/taskmanager.h"


void M_Code_M104(void) 
{//M104 if Sarg, it is target temperature, if Barg it means to emulate pressing the corresponding button from the enum
	//Button arg preempts Sarg action
	//M104 S55 will set the desired temperature to 55c
	//m104 B1 will execute as if xmit had been pressed
	int temp = systemconfig.pcnt.programmed_temperature;
    ESP_LOGI(TAG, "M_Code_M104");//record m104 is received
	//
	if (ARG_B_PRESENT)
	{
		switch ((int)ARG_B)
		{
//		case  BTN_FUNCS:
//			break;
//		case BTN_CLEAR:
//			break;
		case BTN_TEMP_ONOFF:
			systemconfig.pcnt.enabled = systemconfig.pcnt.enabled == 1 ? 0 : 1;
//			systemconfig.pcnt.enabled = !systemconfig.pcnt.enabled;
			break;
		case BTN_TEMP_DECREASE:
			temp -= 1;
			break;
		case BTN_TEMP_INCREASE:
			temp += 1;
			break;
		}
		if (temp < 0) temp = 0;
		else if (temp > 300) temp = 300;
		systemconfig.pcnt.programmed_temperature = temp;
		refreshRequest = true;
		return;
			
	}
    if (ARG_S_MISSING) 
    { //need a S arg, or report it in log box
        ESP_LOGI(TAG, "M_Code_M104: Missing S Argment");
        return;
    }
	if (ARG_S > 0)
	{	
		systemconfig.pcnt.programmed_temperature = (uint16_t)ARG_S;
		systemconfig.pcnt.enabled = true;
		refreshRequest = true;
		HeartBeat = 0;
		return;
	}
	systemconfig.pcnt.enabled = false;
	refreshRequest = true;

}