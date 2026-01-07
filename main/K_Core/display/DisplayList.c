#include "DisplayList.h"
#include "K_Core/taskmanager.h"
#include "K_Core/K_Core.h"
#include "L_Core/ui/ui.h"
#include "L_Core/ui/ui-opc.h"
#include "K_Core/adc/adc.h"
#include "RevisionHistory.h"
#include "K_Core/pcnt/pcnt.h"

int DisplayIndex = 0;

DisplayVariableInfo LcdVarsTable[] = {
//	{ &HeartBeat, "HB  =",			FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[0], "RTD0= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL},
//	{ &RTDsampleHistory[1], "RTD1= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[2], "RTD2= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[3], "RTD3= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[4], "RTD4= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &pcnt_info.rtd_volt, "RTDV= ", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &Kfactor,					"Kfactor=", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[2], "BAT2= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[3], "BAT3= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[4], "BAT4= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[5], "BAT5= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[6], "BAT6= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[7], "BAT7= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[8], "BAT8= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[9], "BAT9= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[10], "AVG= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[5], "RTD5= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[6], "RTD6= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[7], "RTD7= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
//	{ &RTDsampleHistory[8], "RTD8= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },

	//{ &pcnt_info.rtd_volt, "RTDV= ", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
		
	{0},
};


DisplayVariableInfo Lcd1VarsTable[] = {
	{ &HeartBeat, "HB  =", FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[0], "BAT0= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[1], "BAT1= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[2], "BAT2= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[3], "BAT3= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[4], "BAT4= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[5], "BAT5= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[6], "BAT6= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[7], "BAT7= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[8], "BAT8= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &BatterysampleHistory[9], "BAT9= ", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ 0 },
};

DisplayVariableInfo Lcd2VarsTable[] = {
	{ &HeartBeat, "HB", FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{0},
};
DisplayVariableInfo Lcd3VarsTable[] = {
	{ &HeartBeat, "HB", FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{0},
};


void ShowNextDisplay()
{
	DisplayIndex++;
	if(DisplayIndex > (NUMBER_OF_DISPLAY_LISTS)) DisplayIndex = 0;
	
}
void ShowPreviousDisplay()
{
	if (DisplayIndex == 0) DisplayIndex = (NUMBER_OF_DISPLAY_LISTS -1);
	else if(DisplayIndex) DisplayIndex --;
}

DisplayVariableInfo* GetActiveDisplay() {
	DisplayVariableInfo* activeDisplay = NULL;
	switch(DisplayIndex) {
	case 0:
		activeDisplay = Lcd1VarsTable;
		break;
	case 1:
		activeDisplay = Lcd1VarsTable;
		break;
	case 2:
		activeDisplay = Lcd2VarsTable;
		break;
	case 3:
		activeDisplay = Lcd3VarsTable;
		break;
	}
	return activeDisplay;
}