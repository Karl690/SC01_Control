
#include "sequencer.h"
#include "cmdprocessor.h"
#include "L_Core/bluetooth/ble.h"
#include "L_Core/ui/ui.h"
#include "L_Core/ui/ui-comm.h"
#include "L_Core/ui/ui-pct.h"
#include "L_Core/ui/ui-pct01.h"
#include "L_Core/ui/ui-bluetooth.h"
#include "L_Core/ui/ui-plot.h"
#include "L_Core/ui/ui-sdcard.h"
#include "L_Core/bluetooth/ble.h"
#include "K_Core/amplifier/amplifier.h"
#include "K_Core/simple/simple.h"
#include "K_Core/secs/secs.h"

uint32_t CommandsInQue = 0;
uint32_t CommandsInUrgentQue = 0;
uint32_t cmd_NextCommandInsertionPointer = 1;
uint32_t cmd_CurrentPointer = 1;
uint32_t cmd_CommandsInQue = 0;
char cmd_CommandsInQueBuffer[SIZE_OF_COMMAND_QUEUE][MAX_COMMAND_LEN];
uint32_t cmd_start_freq = 9300;
uint32_t cmd_stop_freq = 9800;
uint32_t cmd_freq_inc = 100;

char cmdproc_temp[256];
char lineIndexBuffer[5];
CMD_REPORT_INFO cmd_report_que[0x10];
Meg_Status_INFO CurrentAmpStatus;
uint32_t cmd_report_tail = 0;
uint32_t cmd_report_head = 0;
bool cmd_tuning = false;
bool cmd_sdcard_write_flag = false;
uint16_t downloadedProgrammedTime = 0;
uint16_t downloadedProgrammedPower = 0;

bool cmd_sending_log = false;
FILE* cmd_log_fp = NULL;

uint32_t NextCommandInsertionPointer=1;



int findOfStartOnString(char* cmd)
{
	char* temp = strstr(cmd, "*"); // cmd.IndexOf("*");
	if (!temp) return -1;
	int index = temp - cmd;
	return index;
}

void cmd_sequener()
{
    if(!cmd_CommandsInQue) return;						//no commands to proces, so return
    char* cmd = &cmd_CommandsInQueBuffer[cmd_CurrentPointer][0];
	int len = strlen(cmd);
	ESP_LOGI(TAG, "CMD: %s", cmd);
	if (cmd[len - 1] == '\n') cmd[len - 1] = '\0'; //remove  '\n'
	parseLineCommandData(cmd);
	ui_comm_add_log(cmd, UI_RECEIVE_COLOR);
    cmd_CurrentPointer ++;
    if(cmd_CurrentPointer >= SIZE_OF_COMMAND_QUEUE) cmd_CurrentPointer = 1;
	cmd_CommandsInQue--;
}
void parseLineCommandData(char* CmdLine)
{//commands come in the format of A*, where A is a letter for the function
	if (strlen(CmdLine) < 3)return; //must have at least 4 characters in cmd
	//if (CmdLine[1] != '*')return;//must have * as second character
	
	
	switch (CmdLine[0])
	{
		//process according the cmd letter, which is the first letter of the line
	case 'A':
		parseActionCommand(CmdLine + 2); //move 2 characters to skip the "A*"
		break;
	case 'R'://generic status
		ParseMegStatusString(CmdLine + 2); //move 2 characters to skip the "R*"
		break;
	case 'r'://tunning status
		parseTunningVarsCommand(CmdLine + 2); //move 2 characters to skip the "R*"
		break;
	case 'P'://lots of working variables, dac, duty, uhp.....
		parseProcessInfoCommand(CmdLine + 2); //move 2 characters to skip the "R*"
		break;
	case 'L': //L#=string
		parseLineTextCommand(CmdLine);
		break;
	case 'l': //l#=color
		parseLineColorCommand(CmdLine);
		break;
	case 'B': //B#=string
		parseButtonTextCommand(CmdLine);
		break;
	case 'b': //b#=color
		parseButtonColorCommand(CmdLine);
		break;
	case 'S': //update the BLE device address
		//if (idx != 99) break;
		updateBleAddress(CmdLine);
		break;
	case 's': // save command to TF-Card
		{
			int index = findOfStartOnString(CmdLine); // find '*'
			if (index == -1) break;
			processSaveToSDCard(CmdLine + index + 1);
			break;
		}
		break;
	case 'X':
		processBleScreenAction(CmdLine);
		break;
	case 'V':
		parseProgramedStatusCommand(CmdLine + 2);
		break;
	case 'D': //D#
		processBleSendLogFile(CmdLine);
		break;
	default:parseActionCommand(CmdLine);
		break;
	}	
}


void parseG6Command(char* cmd)
{
	//G6 B#\n
	//char code = cmd[3];
	int value = atoi(cmd + 4); //go to place of Number
	switch (value)
	{
	case 18: //transform comm  screen
		ui_transform_screen(SCREEN_COMM);	
		break;
	case 19: //transform home screen
		ui_transform_screen(SCREEN_HOME);
		break;
	case 99: //G6 S99 ; this is a request to update the ble name from 407		
		break;
	default:
		communication_tx_commandline(MegComPort, cmd);
		break;
	}
}


//parse action string
// A*T#,StartFq,StopFqB, FreqInc
// A*T255
void parseActionCommand(char* cmd)
{
	strcpy(cmdproc_temp, cmd);
	const char s[2] = " ";
	char* token =  strtok(cmdproc_temp, s);
	int index = 0;
	int tArg = 0;
	while (token != NULL) {
		switch (index)
		{
		case 0:
			if (token[0] != 'T') return;
			tArg = atoi(token + 1);
			if (tArg == 255)cmd_tuning = false;
			break;
		case 1:
			if (tArg == 254) cmd_start_freq = atoi(token);
			break;
		case 2:
			if (tArg == 254) cmd_stop_freq = atoi(token);
			break;
		case 3:
			if (tArg == 254) {
				cmd_freq_inc = atoi(token);
				cmd_tuning = true;
				ui_plot_clear();
			}
			break;
		default:
			break;
		}
		index++;
		token = strtok(NULL, s);
	}
	//ui_plot_button_status(cmd_turning);	
}

//parse report string
// R*T# canaddress process_timer CalculatedTotalPower PWR1 Pwr2 Pwr3 Pwr4 Pwr5 Pwr6 Pluggedin\n
void ParseMegStatusString(char* cmd)
{
	strcpy(cmdproc_temp, cmd);
	const char s[2] = " ";//define the delimiting character
	char* token =  strtok(cmdproc_temp, s);//split the string,
	bool is_validate = false;
	int index = 0;
	while (token != NULL) {
		switch (index)
		{
		case 0:
			if (token[0] != 'T') return;
			CurrentAmpStatus.channel = atoi(token + 1); //remove T and convert to integer
			break;
		case 1:
			CurrentAmpStatus.Process_Timer = atoi(token);
			break;
		case 2:
			CurrentAmpStatus.TotalPower = atoi(token);
			break;
		case 3:
			CurrentAmpStatus.Pwr1  = atoi(token);
			break;
		case 4:
			CurrentAmpStatus.Pwr2 = atoi(token);
			break;
		case 5:
			CurrentAmpStatus.Pwr3 = atoi(token);
			break;
		case 6:
			CurrentAmpStatus.Pwr4 = atoi(token);
			break;
		case 7:
			CurrentAmpStatus.Pwr5 = atoi(token);
			break;
		case 8:
			CurrentAmpStatus.Pwr6 = atoi(token);
			break;
		case 9:
			CurrentAmpStatus.array_plugged_in = atoi(token);
			break;
		default:
			break;
		}
		index++;
		token = strtok(NULL, s);//get the next value string
	}
}


// r*T##
void parseTunningVarsCommand(char* cmd)
{
//	int index = 1;//point to first REAL value in the status string
	uint32_t ActualFrequency=9999;
	uint32_t ForwordPower=0;
	
	strcpy(cmdproc_temp, cmd);//make working copy
	const char s[2] = " "; //define the delimiting character
	char* token =  strtok(cmdproc_temp, s); //split the string,

	if (token[0] != 'T')return;//needs to start with a T
	uint8_t CmdCanAddress = atoi(token + 1);
	if (CmdCanAddress > 23)	{	CmdCanAddress -= 24; }
	else if(CmdCanAddress > 17)	{	CmdCanAddress -= 18; }
	else if(CmdCanAddress > 11)	{	CmdCanAddress -= 12; }
	else if(CmdCanAddress > 5)	{	CmdCanAddress -= 6; }
	//at this point we have the channel address
	//however we need to get the OFFset for the channel, 
//	CmdCanAddress -= amplifier.Address;//adjust the offset for this amplifier
	//ui_plot_scan_points[UI_PLOT_CHANNEL_NUM][UI_PLOT_MAX_POINTS];
	if (CmdCanAddress > 5)return;//out of range
	ActualFrequency =  atoi(strtok(NULL, s)); //get first fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get first fequency
	AddPointToXYPlot(CmdCanAddress,ActualFrequency,ForwordPower);//update xy point array
//
	ActualFrequency =  atoi(strtok(NULL, s)); //get second fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get second power
	AddPointToXYPlot(CmdCanAddress+1, ActualFrequency, ForwordPower); //update xy point array
//
	ActualFrequency =  atoi(strtok(NULL, s)); //get 3th fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get 3th power
	AddPointToXYPlot(CmdCanAddress + 2, ActualFrequency, ForwordPower); //update xy point array
//
	ActualFrequency =  atoi(strtok(NULL, s)); //get fourth fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get fourth power
	AddPointToXYPlot(CmdCanAddress + 3, ActualFrequency, ForwordPower); //update xy point array
//
	ActualFrequency =  atoi(strtok(NULL, s)); //get fifth fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get fifth power
	AddPointToXYPlot(CmdCanAddress + 4, ActualFrequency, ForwordPower); //update xy point array

	ActualFrequency =  atoi(strtok(NULL, s)); //get 6th fequency
	ForwordPower =  atoi(strtok(NULL, s)); //get 6th fequency
	AddPointToXYPlot(CmdCanAddress + 5, ActualFrequency, ForwordPower); //update xy point array
}

void AddPointToXYPlot(uint8_t CmdCanAddress, uint32_t ActualFrequency, uint32_t ForwordPower)
{//this method will plug in the forward power , into the 
//	ui_plot_add_channel_value(CmdCanAddress, ActualFrequency, ForwordPower);
	
	CMD_REPORT_INFO* info = &cmd_report_que[cmd_report_head];
	info->chanel = CmdCanAddress;
	info->actual_freq = ActualFrequency;
	info->forward_power = ForwordPower;
	amplifier.RF_Channels[info->chanel].CanAddress = CmdCanAddress;
	amplifier.RF_Channels[info->chanel].vars.ActualFrequency = info->actual_freq;
	amplifier.RF_Channels[info->chanel].vars.ForwordPower = info->forward_power;	
	cmd_report_head++;
	cmd_report_head &= 0xf;
}
//parse process information string
// P*T# PROGRAMMED_POWER1 PROCESS_TIME UHP_Voltage_Control_Duty PROCESS_TIMER ProcessMode
// A*T255
void parseProcessInfoCommand(char* cmd)
{
	strcpy(cmdproc_temp, cmd);
	const char s[2] = " ";
	char* token =  strtok(cmdproc_temp, s);
	bool is_passed = false;
	
	int index = 0;
	int tArg = 0;
	while (token != NULL) {
		switch (index)
		{
		case 0:
			if (token[0] != 'T') return;
			tArg = atoi(token + 1);
			if (tArg >= MAX_CHANNELS) return;
			ampProcessInfo.Channel = tArg;
			break;
		case 1:
			ampProcessInfo.PROGRAMMED_POWER1 = atoi(token);
			break;
		case 2:
			ampProcessInfo.PROCESS_TIME = atoi(token);
			break;
		case 3:
			ampProcessInfo.UHP_Voltage_Control_Duty = atoi(token);
			break;
		case 4:
			ampProcessInfo.PROCESS_TIMER = atoi(token);
			break;
		case 5:
			ampProcessInfo.ProcessMode = atoi(token);
			is_passed = true;
			break;
		}
		index++;
		token = strtok(NULL, s);
	}
	if (systemconfig.serial2.mode == SERIAL2_MODE_SECS)
	{
		//SECS mode
		if (is_passed) 
		{
			// send s1f6 command.
			s1f6message[15] = ampProcessInfo.Channel; // channel
			s1f6message[16] = (uint8_t)ampProcessInfo.ProcessMode; // Status of Meg407
			s1f6message[17] = (uint8_t)0; // Error of Meg407
			s1f6message[20] = (uint8_t)(ampProcessInfo.PROGRAMMED_POWER1 >> 8) & 0x00ff; // high bit of Power
			s1f6message[21] = (uint8_t)(ampProcessInfo.PROGRAMMED_POWER1 & 0x00ff); // high bit of Power
			s1f6message[22] = (uint8_t)(ampProcessInfo.PROCESS_TIME >> 8) & 0x00ff; // high bit of Time
			s1f6message[23] = (uint8_t)(ampProcessInfo.PROCESS_TIME & 0x00ff); // high bit of Time	
		}
		else
		{
			// send s1f6 command.
			s1f6message[15] = ampProcessInfo.Channel; // channel
			s1f6message[16] = (uint8_t)ampProcessInfo.ProcessMode; // Status of Meg407
			s1f6message[17] = (uint8_t)128; // Error of Meg407
		}
		SendSecsCommand(s1f6message, sizeof(s1f6message));	
	}
}

// parse programmed status string
// V*T# PROGPOWER1 PROGFREQ1 PROGWOBBLE1 FREELOCK PROGPOWER2 PROGFREQ2 PROGWOBBLE2 SPAREDUMMY1 DESIREDTIMESEC TUNEFLAG..... 
void parseProgramedStatusCommand(char* cmd)
{
	strcpy(cmdproc_temp, cmd);
	const char s[2] = " ";
	char* token =  strtok(cmdproc_temp, s);
	bool is_passed = false;
	
	int index = 0;
	int tArg = 0;
	AmplifierChannelStruct* channel = NULL;
	while (token != NULL) {
		switch (index)
		{
		case 0:
			if (token[0] != 'T') return;
			tArg = atoi(token + 1);
			if (tArg >= MAX_CHANNELS) return;
			channel = &amplifier.RF_Channels[tArg];
			break;
		case 1:
			if (!channel) break;
			channel->settings.ProgPower1 = atoi(token);
			if (tArg == 0)
			{downloadedProgrammedPower = channel->settings.ProgPower1;}
			break;
		case 2:
			if (!channel) break;
			channel->settings.ProgFrequency1 = atoi(token);
			break;
		case 3:
			if (!channel) break;
			channel->settings.ProgWobble1 = atoi(token);
			break;
		case 4:
			if (!channel) break;
			channel->settings.FreqLock = atoi(token);
			break;
		case 5:
			if (!channel) break;
			channel->settings.ProgPower2 = atoi(token);
			break;
		case 6:
			if (!channel) break;
			channel->settings.ProgFrequency2 = atoi(token);
			break;
		case 7:
			if (!channel) break;
			channel->settings.ProgWobble2 = atoi(token);
			break;
		case 8:
			if (!channel) break;
			if (tArg == 0)
			{downloadedProgrammedTime = atoi(token); }
			//channel->settings.spareDummy1 = atoi(token);
			break;
		case 9:
			if (!channel) break;
			channel->settings.DesiredTimeSec = atoi(token);
			break;
		case 10:
			if (!channel) break;
			channel->settings.TuneFlag = atoi(token);
			break;
		case 11:
			if (!channel) break;
			channel->settings.spareDummy2 = atoi(token);
			break;
		case 12:
			if (!channel) break;
			channel->settings.spareDummy3 = atoi(token);
			break;
		case 13:
			if (!channel) break;
			channel->settings.TunePower1 = atoi(token);
			break;
		case 14:
			if (!channel) break;
			channel->settings.TuneFreqStart1 = atoi(token);
			break;
		case 15:
			if (!channel) break;
			channel->settings.TuneFreqStop1 = atoi(token);
			break;
		case 16:
			if (!channel) break;
			channel->settings.TuneVelocity1 = atoi(token);
			break;
		case 17:
			if (!channel) break;
			channel->settings.TunePower2 = atoi(token);
			break;
		case 18:
			if (!channel) break;
			channel->settings.TuneFreqStart2 = atoi(token);
			break;
		case 19:
			if (!channel) break;
			channel->settings.TuneFreqStop2 = atoi(token);
			break;
		case 20:
			if (!channel) break;
			channel->settings.TuneVelocity2 = atoi(token);
			is_passed = true;
			break;
		}
		index++;
		token = strtok(NULL, s);
	}
	
	if (is_passed)
	{
		// if the V* command from Meg407 is parsed , it should send the Dx command to Host. 
		/*the legacy format for the download recipe, MEG>>MEGPI, 5 variables total
			 *programmed Time
			 *programmed Power
			 *desired temperature
			 *min temperature
			 *max temperature
			 **/
		sprintf(temp_string, "D0,%d,%d,0,0,0\n", downloadedProgrammedTime, downloadedProgrammedPower);
		SendStringToPLC(temp_string);
	}
}

// s*[string to save]
void processSaveToSDCard(char* cmd)
{
	if (!systemconfig.sdcard.status) return;
	
	sprintf(ui_temp_string, SDCARD_MOUNT_POINT "/meg407.txt");
	
	FILE* fp = fopen(ui_temp_string, "a+");
	
	if (fp)
	{
		sprintf(ui_temp_string, "%s\n", cmd);
		fwrite(ui_temp_string, 1, strlen(ui_temp_string), fp);
		fclose(fp);
		cmd_sdcard_write_flag = true;
	}
	else
	{
		ui_show_messagebox(MESSAGEBOX_ERROR, "Can't open file: meg407.txt", 3000);
		
	}
}
void parseLineTextCommand(char* cmd)
{
	int index = findOfStartOnString(cmd);
	if (index == -1) return;
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	if (idx < 10) ui_pct_update_label_text(idx, cmd + index + 1);
	else ui_pct01_update_label_text(idx - 10, cmd + index +1);
}

void parseLineColorCommand(char* cmd)
{
	int index = findOfStartOnString(cmd);
	if (index == -1) return;
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	if (idx < 10) ui_pct_update_label_color(idx, cmd + index + 1);
	else ui_pct01_update_label_color(idx - 10, cmd + index + 1);
}

void parseButtonTextCommand(char* cmd)
{
	int index = findOfStartOnString(cmd);
	if (index == -1) return;
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	ui_pct_update_button_text(idx, cmd + index + 1);
	//else ui_pct01_update_button_text(idx - 10, cmd + index + 1);
}
void parseButtonColorCommand(char* cmd)
{
	int index = findOfStartOnString(cmd);
	if (index == -1) return;
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	ui_pct_update_button_color(idx, cmd + index + 1);
	//else ui_pct01_update_button_color(idx - 10, cmd + index + 1);
}

void updateBleAddress(char* cmd)
{
	if (cmd[1] != '9') return;
	if (cmd[2] != '9') return;
	int address = atoi(cmd + 4);//S99*##   ## is the channel address
	ble_update_name(address);
	systemconfig.can_address =(uint16_t) address;//update for sending messages to 407
	save_configuration();
}

void processBleScreenAction(char* cmd)
{
	int index = findOfStartOnString(cmd);
	if (index == -1) return;
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	
	uint8_t screen_id, button_id;
	screen_id = idx;
	button_id = atoi(cmd + index + 1);
	
	ui_request_update = true;
	ui_request_screen_id = screen_id;
	ui_request_button_id = button_id;
}

void cmd_transfer_log_file_task()
{
	if (!cmd_sending_log) return;
	if (fgets(temp_string, 256, cmd_log_fp) != NULL)
	{
		// Remove the newline character if present
		temp_string[strcspn(temp_string, "\n")] = '\0'; // Replace newline with null-terminator
		ble_server_send_data((uint8_t*)temp_string, strlen(temp_string));
		vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 1000ms (1 second)
	}
	else
	{
		fclose(cmd_log_fp);	
		cmd_log_fp = NULL;
		cmd_sending_log = false;
	}
}

void processBleSendLogFile(char* cmd)
{
	int index = findOfStartOnString(cmd);
	char szT[5] = { 0 };
	strncpy(szT, cmd + 1, index - 1);
	int idx = atoi(szT);
	if (idx == -1) return;
	// sprintf(ui_temp_string, SDCARD_MOUNT_POINT "/TuneHistory.txt");
	char* opened_file = ui_sdcard_get_opened_filepath();
	if (!opened_file) {
		strcpy(temp_string, "No file is open.\n");
		ble_server_send_data((uint8_t*)temp_string, strlen(temp_string));
		return;
	}
	switch (idx)
	{
	case 0: // send log file
		cmd_sending_log = false;
		cmd_log_fp = fopen(opened_file, "r");
		if (!cmd_log_fp) {
			strcpy(temp_string, "Open file failed.\n");
			ble_server_send_data((uint8_t*)temp_string, strlen(temp_string));
			return;
		}
		cmd_sending_log = true;
		break;
	case 1:
		// remove(opened_file);	
		{
			FILE* fp = fopen(opened_file, "w");
			if (fp) fclose(fp);
			strcpy(temp_string, "The file has been cleaned up.\n");
			ble_server_send_data((uint8_t*)temp_string, strlen(temp_string));
			ui_sdcard_open_file(opened_file);
		}
		break;
	}
}


void processArgs(char *WorkBuffer,float *OutPutVariable)
{
	if (*WorkBuffer == 0)return;//if first chacter is null, return

	//WorkBuffer++;//MOVE OVER 1 CHAR SO WE CAN PROCESS THE NUMBER, NOT THE KEY LETTER
	if (*WorkBuffer==0)
	{   // no value to convert
		// ProcessingError = 1;
		*OutPutVariable = INVALID_ARG_VALUE;//set to invalid value so we will not accidentally take a zero as a position or temperature argument
		return;
	}
	WorkBuffer++;//point to second charcter please, the first is the key character
	*OutPutVariable =(float) atof(WorkBuffer);//start with the second character in the string, because the first character is the argument header char, like M or G or X  etc.
}