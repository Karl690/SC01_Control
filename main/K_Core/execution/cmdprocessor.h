#pragma once
#include "main.h"
#include "sequencer.h"

#define COMMENT_STRING_LENGTH   	(256+3)	//+3 for leading and trailing delim + NULL_CHAR
#define ARG_PRESENT(arg)   ((uint16_t)arg != 0xffff)
typedef enum
{
	BLE_FUNC_UNKNOWN = 0,
	BLE_FUNC_PING = 1,
	BLE_FUNC_READ_SOAPSTRING = 2,
}BLE_FUNC_T;


typedef struct
{
	uint8_t chanel;
	uint32_t actual_freq;
	uint32_t dac;
	uint32_t forward_power;
	uint32_t refected_power;
	uint32_t array_plugin;
	uint32_t process_mode;
	char	major_step[10];
}
CMD_REPORT_INFO;
typedef struct
{
	uint16_t channel;
	uint16_t Process_Timer;
	uint16_t TotalPower;
	uint16_t Pwr1;
	uint16_t Pwr2;
	uint16_t Pwr3;
	uint16_t Pwr4;
	uint16_t Pwr5;
	uint16_t Pwr6;
	uint16_t array_plugged_in;
}
Meg_Status_INFO;

typedef struct
{
	uint8_t CanAddress;
	uint32_t ActualFrequency1;
	uint32_t ForwordPower1;
	uint32_t ActualFrequency2;
	uint32_t ForwordPower2;
	uint32_t ActualFrequency3;
	uint32_t ForwordPower3;
	uint32_t ActualFrequency4;
	uint32_t ForwordPower4;
	uint32_t ActualFrequency5;
	uint32_t ForwordPower5;
	uint32_t ActualFrequency6;
	uint32_t ForwordPower6;
	//uint32_t array_plugin;
}
Tunning_Status_String;

extern uint32_t cmd_NextCommandInsertionPointer;
extern uint32_t cmd_CommandsInQue;

extern uint32_t cmd_start_freq;
extern uint32_t cmd_stop_freq;
extern uint32_t cmd_freq_inc;
extern bool cmd_tuning;
extern CMD_REPORT_INFO cmd_report_que[0x10];
extern Meg_Status_INFO CurrentAmpStatus;
extern uint32_t cmd_report_tail;
extern uint32_t cmd_report_head;
extern uint16_t downloadedProgrammedTime;
extern uint16_t downloadedProgrammedPower;
extern bool cmd_sdcard_write_flag;

extern uint32_t CommandsInQue;
extern uint32_t CommandsInUrgentQue;
extern char cmd_CommandsInQueBuffer[SIZE_OF_COMMAND_QUEUE][MAX_COMMAND_LEN];

extern uint32_t NextCommandInsertionPointer;


void cmd_sequener();
void parseLineCommandData(char* cmd);
uint8_t convertCommandData(char*);
void parseG6Command(char* cmd);

void updateBleAddress(char* cmd);
void parseActionCommand(char* cmd);
void ParseMegStatusString(char* cmd);
void parseTunningVarsCommand(char* cmd);
void AddPointToXYPlot(uint8_t CmdCanAddress, uint32_t ActualFrequency, uint32_t ForwordPower);

void parseProcessInfoCommand(char* cmd);
void parseLineTextCommand(char* cmd);
void parseLineColorCommand(char* cmd);
void parseButtonTextCommand(char* cmd);
void parseButtonColorCommand(char* cmd);
void processBleScreenAction(char* cmd);
void parseProgramedStatusCommand(char* cmd);
void processSaveToSDCard(char* cmd);

void cmd_transfer_log_file_task();
void processBleSendLogFile(char* cmd);

void processArgs(char *WorkBuffer,float *OutPutVariable);