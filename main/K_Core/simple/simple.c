#include "main.h"
#include "../serial/serial.h"
#include "simple.h"
#include "L_Core/devices/display.h"
#include "L_Core/ui/ui.h"
#include "L_Core/ui/ui-simple.h"
#include "K_Core/amplifier/amplifier.h"
#include "K_Core/execution/cmdprocessor.h"
SIMPLE_OBJ simple_obj;
GENERATOR_STATUS simple_generator_status[SIMPLE_GENERATOR_NUM];
uint16_t simple_buffer_pos = 0;
char ui_simple_temp[1024] = { 0 };
uint8_t ui_simple_cmd[10];
uint32_t simple_dump_display_address = 0;
bool simple_dump_captured = false;


uint8_t simple_command_start_logging[3] = { 'L', '0', CMD_END_CHAR  };
uint8_t simple_command_stop_logging[3] = { 'l', '0', CMD_END_CHAR };
uint8_t simple_command_query[3] = { 'Q', '1', CMD_END_CHAR };
uint8_t simple_command_update_recipe[3] = { 'U', '1', CMD_END_CHAR };
uint8_t simple_command_download_recipe[3] = { 'D', '0', CMD_END_CHAR };
uint8_t simple_command_start_process[3] = { 'O', '0', CMD_END_CHAR };
uint8_t simple_command_cancel_process[3] = { 'o', '0', CMD_END_CHAR };
uint8_t simple_command_identify[3] = { 'I', '0', CMD_END_CHAR };

void simple_init()
{
	memset(&simple_obj, 0, sizeof(SIMPLE_OBJ)); //reset memory space
	memset(simple_generator_status, 0, SIMPLE_GENERATOR_NUM* sizeof(SIMPLE_OBJ));
	simple_obj.serial = &ComUart2;
}
/*

*/

/*this class is meant to facilitate simple serial communication between a HOST, (PLC or PC)
 *and the amplifier. it has the sco1 configured as a smart display
 *it handles the incoming simple serial command strings
 *and it decides what to do with the commands,
 *the sco1 uses;
 *uart1 <> Meg407 default baud rate is 115200
 *uart2 <> HOST  default baud rate is 9600
 *
 **/
void simple_send_command(uint8_t code)
{
	if (systemconfig.serial2.mode != SERIAL2_MODE_SIMPLE) return; //mode:0 SIMPLE, 1: SECS;  	
	uint8_t* working_command = NULL;
	uint16_t len = 0;
	if (!simple_obj.serial)return;//can not send to invalid port
	switch (code)
	{
	case SIMPLE_CMD_PING:
		simple_send_ping();
		break;
	case SIMPLE_CMD_START_LOGGING:
		SendStringToPLCLog("L0\n");		//start the logging
		break;
	case SIMPLE_CMD_STOP_LOGGING:
		SendStringToPLCLog("l0\n"); //stop the logging
		break;
	case SIMPLE_CMD_QUERY:
		SendStringToPLCLog("Q0\n"); //Query status
		break;
	case SIMPLE_CMD_UPDATE_RECIPE:
		//karllvana   this is broken, need to form the correct string   U0,time,power \n
		//working_command = simple_command_update_recipe; len = 3;
		break;
	case SIMPLE_CMD_DOWNLOAD_RECIPE:
		SendStringToPLCLog("D0\n"); //start download for recipe
		break;
	case SIMPLE_CMD_START_PROCESS:
		SendStringToPLCLog("O0\n"); //start the process
		break;
	case SIMPLE_CMD_CANCLE_PROCESS:
		SendStringToPLCLog("o0\n"); //STOP process, not the same as CANCEL all , which is "X0\n"
		break;
	case SIMPLE_CMD_IDENTIFY:
		SendStringToPLCLog("I0\n");//request identity loop, to confirm how many units are in loop
		break;
	}
}


void ParseIncommingLineToSimpleString()
{
	if (!simple_obj.serial) return;
	if (simple_obj.serial->RxBuffer.Head == simple_obj.serial->RxBuffer.Tail)return;//nothing to 
	char WorkRxChar;
	ComBuffer* SourceBuff = &simple_obj.serial->RxBuffer;
	COMPORT* WorkingComPort = simple_obj.serial;
	char* Command_buffer = simple_obj.Cmd_que_buffer[simple_obj.head];
	for (uint16_t i = 0; i < PROCESS_MAX_CHARS_TO_READ_ON_ONE_SLICE; i++)
	{
		if (SourceBuff->Head == SourceBuff->Tail) break;//do not process characters that are not there.
		WorkRxChar = SourceBuff->buffer[SourceBuff->Tail];//get the next character
		SourceBuff->Tail++; //point to the next character for next time we need one.
		SourceBuff->Tail &= (SourceBuff->Buffer_Size - 1); //circular que with even hex size....
		if (WorkRxChar  > 0x19 && WorkRxChar <= 0x7F)
		{//if normal ascii character, simply pass onto command buffer.
			Command_buffer[simple_buffer_pos] = WorkRxChar;//xfer character to the cmd buffer
			simple_buffer_pos++;//should be limit check on buffer size
			continue;
		}
//		else if (WorkRxChar == CMD_END_CHAR || simple_buffer_pos + 1 >= SIMPLE_CMD_MAX_LEN)
//		{
//			if (simple_buffer_pos == 0) break;//check for null command string, do not process null
//			Command_buffer[simple_buffer_pos] = 0;//set END of command line to null, terminate the cmdstring
//			//simple_obj.head++;
//			simple_buffer_pos = 0;//reset the pointer
//			simple_obj.head = simple_obj.head >= SIMPLE_CMD_QUE_SIZE ? 0 : simple_obj.head + 1; //0xf
//			break;
//		}
//		else
//		{
			switch (WorkRxChar)
			{
			
			case PING_REPLY:
				if (!dump_display_sending)
				{
					WorkingComPort->TxAcknowledgeCounter--; //keep track of how far behind we are
				}
				if (WorkingComPort->TxAcknowledgeCounter < 0) WorkingComPort->TxAcknowledgeCounter = 0; //in case of underrun from reset condition
				if (WorkingComPort->pingSent)
				{
					ui_simple_add_char(PING_REPLY, UI_RECEIVE_COLOR);	
					WorkingComPort->pingSent = false;
				}
				break;
			case PING_CHAR:
				WorkingComPort->RxAcknowledgeCounter = 1; // replay 0x6 only at one time.
				WorkingComPort->TxAcknowledgeCounter = 0;
				if (simple_dump_captured)
				{
					display_reset_capture_buffer(); // reset capture buffer.
					simple_dump_captured = false;
				}
				else
				{
					ui_simple_add_char(PING_CHAR, UI_RECEIVE_COLOR);	
					ui_simple_add_char(PING_REPLY, UI_SEND_COLOR);		
				}
				break;
			case CR_CHAR:
			case CMD_END_CHAR:  //if (rawChar==10) 0xA or 0xD  can trigger the end of line		
				if (simple_buffer_pos <= 1)
				{
					//
					// if command length is 0, do nothing. 
					// for example, aaa 0xa 0xd, and WorkRxChar is CR_CHAR, command line' length should be 1
					// finally, WorkingComPort->CommandLineBuffer = '\0' or WorkingComPort->CommandLineBuffer = '\r'
					return;
				}
				Command_buffer[simple_buffer_pos] = 0; //set END of command line to null, terminate the cmdstring
				//simple_obj.head++;
				simple_buffer_pos = 0; //reset the pointer
				simple_obj.head = simple_obj.head >= SIMPLE_CMD_QUE_SIZE ? 0 : simple_obj.head + 1; //0xf
				break;
			}	
		}
	//}
}

void simple_responsive_identifier(char* command)
{
	
}

void simple_send_ping()
{
	communication_add_char_to_serial_buffer(&simple_obj.serial->TxBuffer, PING_CHAR);	
	simple_obj.serial->pingSent = true;
	ui_simple_add_line("Send ping 0x7", UI_SEND_COLOR, false);
}
void simple_parse_generator_status(char* command)
{
	char szTemp[100] = { 0 };
	strcpy(szTemp, command);
	char* token = strtok(szTemp, ",");
	bool is_parsed = false;
	GENERATOR_STATUS g = { 0};
	for (int i = 0; i < 11; i++)
	{	
		if (!token) break;
		switch (i)
		{
		case 0:
			g.ampUnit = atoi(token);
			break;
		case 1:
			g.channel = atoi(token);
			if (g.channel >= 8) return;
			break;
		case 2:
			g.freq1 = atoi(token);
			break;
		case 3:
			g.tc6 = atoi(token);
			break;
		case 4:
			g.freq2= atoi(token);
			break;
		case 5:
			g.power1 = atoi(token);
			break;
		case 6:
			g.power2 = atoi(token);
			break;
		case 7:
			g.bathTemp = atoi(token);
			break;
		case 8:
			g.tc1 = atoi(token);
			break;
		case 9:
			g.tc2 = atoi(token);
			break;
		case 10:
			g.status = atoi(token);
			break;
		}
	}
	if (is_parsed)
	{
		memcpy(&simple_generator_status[g.channel], &g, sizeof(GENERATOR_STATUS));
	}
}

void simple_send_dump_screen()
{
	if (!dump_display_sending) {
		if (dump_display_waiting > 1) dump_display_waiting--;
		else if(dump_display_waiting == 1)
		{
			uart_write_bytes((uart_port_t)ComUart2.uart_id, "DC\n", 3); //Complete = C
			dump_display_waiting--;
		}
		return;
	}
	if (dump_display_waiting > 0)
	{
		dump_display_waiting--;
		return;
	}
	uint32_t address = simple_dump_display_address;
	if (address >= display_compress_buffer_size) {
		dump_display_waiting = WAITING_VALUE;
		dump_display_sending = false;
		return;
	}
	memset(ui_simple_temp, 0, 1024);

	size_t bytes = SCREEN_DUMP_SIZE;
	if (simple_dump_display_address + SCREEN_DUMP_SIZE >= display_compress_buffer_size)
	{
		bytes = display_compress_buffer_size - simple_dump_display_address;
	}
	uart_write_bytes((uart_port_t)ComUart2.uart_id, display_snapshot_compress_buffer + address, bytes);
	simple_dump_display_address += bytes;
	sprintf(temp_string, "%d/%d bytes sent", (int)simple_dump_display_address, display_compress_buffer_size);
	ui_simple_add_log(temp_string, UI_SEND_COLOR);
}
void simple_stop_dump_sending()
{
	dump_display_sending = false;
}

//need to upload the recipe to main amplifier module at T0
//data is in the format U0,time,Power,temp,blah,blah in seconds 0-9999
//only support process time, and process power

void simple_upload_ProcessVariables(char* command)
{
	strcpy(temp_string, command);
	const char s[2] = ",";
	int power = 0;
	int process_time = 0;
	int address = 0;
	char* token =  strtok(temp_string, s);
	int index = 0;
	
	while (token != NULL) {
		switch (index)
		{
		case 0:
			address = atoi(token);
			break;
		case 1:
			process_time = atoi(token);
			break;
		case 2:
			power = atoi(token);
			break;
		default:
			break;
		}
		index++;
		token = strtok(NULL, s);
	}
	if (index < 3) // invalid command
	{
		return;
	}
	//now we have the power and time, lets send it to the Meg407 modules
	sprintf(temp_string, "M809 T%d P%d S%d\n", 0, power, process_time);
	SendStringToMeg407(temp_string);
}
	
void SendStringToPLC(char* SourceString)
{SendString(&simple_obj.serial->TxBuffer, SourceString, false); }

void SendStringToPLCLog(char* SourceString)
	{SendString(&simple_obj.serial->TxBuffer, SourceString, true); }

void SendStringToMeg407(char* SourceString)
{SendString(&MegComPort->TxBuffer, SourceString, false);}

void SendStringToMeg407Log(char* SourceString)
	{SendString(&MegComPort->TxBuffer, SourceString, true); }

void SendString(ComBuffer *targetBuffer, char* SourceString, bool Log)
{
	//sends a command string to the meg407 , first module in amplifier serial
	//ComBuffer *targetBuffer = &MasterCommPort->TxBuffer;
	uint16_t size = strlen(SourceString);
	uint16_t index = 0;
	for (index = 0; index < size; index++)
	{
		targetBuffer->buffer[targetBuffer->Head] = SourceString[index];
		targetBuffer->Head++;
		targetBuffer->Head &= (targetBuffer->Buffer_Size - 1);
	}
	if (Log)ui_simple_add_log(SourceString, UI_SEND_COLOR);
}


//SendString
void simple_download_responsive(char* command)
{
	SendStringToMeg407("M809 D1\n");
}
void simple_parse_command()
{
	if (simple_obj.head == simple_obj.tail) return;//check to see if a character is waiting
	char* command = simple_obj.Cmd_que_buffer[simple_obj.tail];//get last character
	
	//simple_obj.Cmd_que_buffer[simple_obj.tail]
	simple_obj.tail++;
	if (simple_obj.tail >= SIMPLE_CMD_QUE_SIZE)simple_obj.tail = 0;//circular que, reset pointer.

	ui_simple_add_log(command, UI_RECEIVE_COLOR);
	uint32_t index = 0;
	char FunctionCharacter = command[0];//get the function, o,X,I,Q,L,I,U,D
	char AddressCharacter = command[1];//get the target amplifier address, 0-9
	
	switch (FunctionCharacter)
	{
		//screen dumping commands, not part of normal simple serial operation, use with caution
	case 'R':
		uart_write_bytes((uart_port_t)ComUart2.uart_id, "DS\n", 3); // Start
		dump_display_waiting = WAITING_VALUE;
		simple_dump_display_address = 0;
		dump_display_sending = true;	
		if (display_dump_buffer())
		{
			simple_dump_captured = true;
			sprintf(temp_string, "DN %d\n", display_compress_buffer_size); //Display Number = DN
			uart_write_bytes((uart_port_t)ComUart2.uart_id, temp_string, strlen(temp_string));
		}
		else
		{
			uart_write_bytes((uart_port_t)ComUart2.uart_id, "DE\n", 3); // ERROR
		}
		break;
	case 'r':
		dump_display_waiting = 0;
		dump_display_sending = false;
		uart_write_bytes((uart_port_t)ComUart2.uart_id, "DU\n", 3); // Display stop by user
		break;
		//end of section for screen dump 		
	case 'L': // Start sending Q string, start automatic logging
		index = atoi(command + 1);
		amplifier_set_logging(index, true);
		SendStringToPLC("L0\n");//pass on command to all amplifiers in the daisy chain loop
		break;
	case 'l': // Stop sending Q string, stop automatic logging
		index = atoi(command + 1);
		amplifier_set_logging(index, false);
		SendStringToPLC("l0\n"); //pass on command to all amplifiers in the daisy chain loop
		break;
	case 'I': //Identy installed amplifiers, echos back if installed
		AddressCharacter++;//increment the address
		sprintf(ui_simple_temp, "I%c\n", AddressCharacter); //		
		SendStringToPLC(ui_simple_temp);
		break;
	case 'Q': 
		if (AddressCharacter == '0')
		{
			//amplifier_build_status_string(); //send the latest status string	
			if (amplifier_logging[0] == 0)
			{
				(amplifier_logging[0] = 0xff);//set the one shot on channel 0 index
				amplifier_channel_index = 0;
				break;
			}
			SendStringToPLC("Q0\n"); //pass on command to all amplifiers in the daisy chain loop
		}	
		break;
	case 'O': // start process
		SendStringToMeg407("M801 I660 T255\n");
		SendStringToPLC("O0\n"); //pass on command to all amplifiers in the daisy chain loop
		break;
	case 'o': // stop
		index = atoi(command + 1);
		if (index > MAX_CHANNELS) return; // invalid

		SendStringToMeg407("M801 I661 T255\n");
		SendStringToPLC("o0\n"); //pass on command to all amplifiers in the daisy chain loop

		break;
	case 'X':
		SendStringToMeg407("M801 I661 T255\n");
		SendStringToPLC("X0\n"); //pass on command to all amplifiers in the daisy chain loop
		break;
	case 'U': //uplaod power and process time
		// for example, HOST<U1,200,300>>SImpleserial>>SCO1>>Mcode P200 S300>>MEG407
		simple_upload_ProcessVariables(command + 1);
		strcat(command, "\n");
		SendStringToPLC(command);
		break;
	case 'D': //set flag to read back the parameters from meg407 primary module for power and time
			//then send that information to the host
		//simple_download_responsive(command + 1);
		//SendStringToPLC("D0\n"); //pass on command to all amplifiers in the daisy chain loop
		SendStringToMeg407("M809 D1\n");
		// sprintf(temp_string, "D0,%d,%d,0,0,0\n", downloadedProgrammedTime, downloadedProgrammedPower);
		/// SendStringToPLC(temp_string);
		break;
	}
}