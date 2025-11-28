#include "DisplayList.h"
#include "K_Core/taskmanager.h"
#include "K_Core/K_Core.h"
#include "L_Core/ui/ui.h"
#include "L_Core/ui/ui-opc.h"
#include "K_Core/adc/adc.h"
#include "RevisionHistory.h"

int DisplayIndex = 0;

DisplayVariableInfo LcdVarsTable[] = {
	{ &HeartBeat, "HB", FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[0].convAvg, "ADC CH0", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[0].adcAvg, "ADC CH0-Raw", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[1].convAvg, "ADC CH1", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[1].adcAvg, "ADC CH1-Raw", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[2].convAvg, "ADC CH2", FUNC_FLOAT, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{ &ADC_Channel[2].adcAvg, "ADC CH2-Raw", FUNC_INT16, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{0},
};


DisplayVariableInfo Lcd1VarsTable[] = {
	{ &HeartBeat, "HB", FUNC_INT32, COLOR_WHITE, COLOR_RED, 0, 0, NULL },
	{0},
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