#include "communication.h"
#include "RevisionHistory.h"
#include "K_Core/serial/serial.h"
#include "K_Core/communication/parser.h"
#include "L_Core/bluetooth/ble.h"
#include "L_Core/ui/ui.h"
#include "L_Core/ui/ui-comm.h"
#include "L_Core/ui/ui-simple.h"
#include "K_Core/secs/secs.h"
#include "K_Core/simple/simple.h"

ComBuffer RawRxComBuffer;
ComBuffer RawRxUrgentComBuffer;
COMPORT* MegComPort = &ComUart1; // &COMUSB; //used to specify which port is the master 
uint8_t tx_check_flag = 0;
uint8_t rx_check_flag = 0;
void communication_buffers_serial_init(uint8_t UartIndex, COMPORT* ComPort, uint8_t* RxBuffer, uint8_t* RxUgrentBuffer, uint8_t* TxBuffer)
{	
	//Initialize Secs serial's buffers
	ComPort->uart_id = UartIndex;
	ComPort->ComType                = COMTYPE_AUX; //primary control port for PC and REPETREL comm
	ComPort->RxBuffer.buffer     	= RxBuffer;
	ComPort->RxBuffer.Buffer_Size  = RX_BUF_SIZE; //this number is used to keep in bounds
	ComPort->RxBuffer.Head          = 0; //start of que
	ComPort->RxBuffer.Tail          = 0; //end of the que
	ComPort->RxBuffer.RxLineCount = 0; // if there is a valid command waiting
	memset(ComPort->RxBuffer.buffer, 0, RX_BUF_SIZE);
	
	ComPort->RxBuffer.commandPtr = ComPort->RxBuffer.command; //reset the command buffer
	
	ComPort->TxBuffer.buffer     	= TxBuffer;
	ComPort->TxBuffer.Buffer_Size  = TX_BUF_SIZE; //this number is used to keep in bounds
	ComPort->TxBuffer.Head		    = 0; // index of where to put the next char
	ComPort->TxBuffer.Tail	        = 0; // index of where to pull the next char
	ComPort->TxBuffer.commandPtr = ComPort->TxBuffer.command; //reset the command buffer
	memset(ComPort->TxBuffer.buffer, 0, TX_BUF_SIZE);	
	
	ComPort->RxUrgentBuffer.buffer      = RxUgrentBuffer;
	ComPort->RxUrgentBuffer.Buffer_Size  = RX_URGENT_BUF_SIZE; //this number is used to keep in bounds
	ComPort->RxUrgentBuffer.Head        = 0; //start of que
	ComPort->RxUrgentBuffer.Tail        = 0; //end of the que
	ComPort->RxUrgentBuffer.RxLineCount = 0; // if there is a valid command waiting
	ComPort->UrgentFlag					= 0;
	ComPort->TxAcknowledgeCounter = 0;
	ComPort->RxAcknowledgeCounter = 0;
	ComPort->CommandLineIdx = 0;
	ComPort->RxUrgentBuffer.commandPtr = ComPort->RxUrgentBuffer.command; //reset the command buffer
	memset(ComPort->RxUrgentBuffer.buffer, 0, RX_BUF_SIZE);
}

void communication_buffers_ble_init(uint8_t id, BleDevice* device, uint8_t* RxBuffer, uint8_t* TxBuffer)
{
	//Initialize Secs serial's buffers
	device->ble_id = id;
	device->RxBuffer.buffer =		RxBuffer;
	device->RxBuffer.Buffer_Size =		RX_BUF_SIZE;
	device->RxBuffer.Head          = 0; //start of que
	device->RxBuffer.Tail          = 0; //end of the que
	device->RxBuffer.commandPtr		= device->RxBuffer.command;
	memset(device->RxBuffer.buffer, 0, device->RxBuffer.Buffer_Size);
	device->AcksWaiting            = 0;
	
	device->TxBuffer.buffer =		TxBuffer;
	device->TxBuffer.Buffer_Size =		TX_BUF_SIZE;
	device->TxBuffer.Head		    = 0; // index of where to put the next char
	device->TxBuffer.Tail	        = 0; // index of where to pull the next char
	device->TxBuffer.commandPtr		= device->TxBuffer.command;
	memset(device->TxBuffer.buffer, 0, device->TxBuffer.Buffer_Size);	
//	
//	device->RxUrgentBuffer.Head        = 0; //start of que
//	device->RxUrgentBuffer.Tail        = 0; //end of the que	
//	device->RxUrgentBuffer.commandPtr		= device->RxUrgentBuffer.command;
//	memset(device->RxUrgentBuffer.buffer, 0, RX_BUF_SIZE);
	device->UrgentFlag					= 0;
	device->AcksWaiting				= 0;
}
void communication_add_buffer_to_serial_buffer(ComBuffer *targetBuffer, uint8_t* buf, uint16_t size)
{
	uint16_t index = 0;
	for (index = 0; index < size; index++)
	{
		targetBuffer->buffer[targetBuffer->Head] = buf[index];
		targetBuffer->Head++;
		targetBuffer->Head &= (targetBuffer->Buffer_Size -1);
	}
}

void communication_add_string_to_serial_buffer(ComBuffer *targetBuffer, char* SourceString)
{
	uint16_t size = strlen(SourceString);
	communication_add_buffer_to_serial_buffer(targetBuffer, (uint8_t*)SourceString, size);
}


void communication_add_char_to_serial_buffer(ComBuffer *targetBuffer, uint8_t RawChar)
{
	targetBuffer->buffer[targetBuffer->Head] = RawChar;
	targetBuffer->Head++;
	targetBuffer->Head &= (targetBuffer->Buffer_Size -1);
}


void communication_add_char_to_ble_buffer(ComBuffer *targetBuffer, uint8_t RawChar)
{
	if (ble_server_status != BLE_SERVER_PAIRED) return;
	targetBuffer->buffer[targetBuffer->Head] = RawChar;
	targetBuffer->Head++;
	targetBuffer->Head &= (targetBuffer->Buffer_Size - 1);
}

void communication_add_buffer_to_ble_buffer(ComBuffer *targetBuffer, uint8_t* buf, uint16_t size)
{
	if (ble_server_status != BLE_SERVER_PAIRED) return;
	uint16_t index = 0;
	for (index = 0; index < size; index++)
	{
		targetBuffer->buffer[targetBuffer->Head] = buf[index];
		targetBuffer->Head++;
		targetBuffer->Head &= (targetBuffer->Buffer_Size - 1);
	}
}
void communication_add_string_to_ble_buffer(ComBuffer *targetBuffer, char* SourceString)
{
	if (ble_server_status != BLE_SERVER_PAIRED) return;
	uint16_t size = strlen(SourceString);
	communication_add_buffer_to_ble_buffer(targetBuffer, (uint8_t*)SourceString, size);
}


void communication_process_rx_serial(COMPORT* WorkingComPort)
{
	if (WorkingComPort->RxBuffer.Head == WorkingComPort->RxBuffer.Tail)return;//nothing to 
	ComBuffer* SourceBuff = &WorkingComPort->RxBuffer;
	uint8_t i = 0;
	char WorkRxChar;
	uint32_t idx = 0; 
	for (i = 0; i < PROCESS_MAX_CHARS_TO_READ_ON_ONE_SLICE; i++)
	//while (SourceBuff->Head != SourceBuff->Tail)
	{
		if (SourceBuff->Head == SourceBuff->Tail) break;
		//must be != because it is circular que.... can not simpley be less than
		WorkRxChar = SourceBuff->buffer[SourceBuff->Tail];
		SourceBuff->Tail++; //point to the next character
		SourceBuff->Tail &= (SourceBuff->Buffer_Size - 1); //circular que with even hex size....

		if ((WorkRxChar  > 0x19 && WorkRxChar <= 0x7F) || WorkRxChar == '\r')
		{
			//normal ascii character processing, below 20 hex are special control characters
			//communication_add_char_to_serial_buffer(TargetBuff, WorkRxChar);
			WorkingComPort->CommandLineBuffer[WorkingComPort->CommandLineIdx] = WorkRxChar;
			WorkingComPort->CommandLineIdx++;
			if (idx >= 255)
			{
				WorkingComPort->CommandLineBuffer[254] = CMD_END_CHAR;
				parser_add_line_to_commandbuffer(WorkingComPort);
				WorkingComPort->UrgentFlag = 0; 
				WorkingComPort->CommandLineIdx = 0;
				memset(WorkingComPort->CommandLineBuffer, 0, 256);
			}
		}
		//if you get here, you must process special characters
		else
		{			
			switch (WorkRxChar)
			{
			case   0:                                       break; //null end of the string
			case TERMINATE_WAIT_CHAR:   		//if (rawChar==1)
				// Process_TERMINATE_WAIT_CHAR(); 
				break;

			case PAUSE_AT_END_OF_MOVE_CHAR:     //if (rawChar==2)
				//requestToPauseAtEndOfMove = 1; 
				break;//set flag to stop at end of this move

			case PAUSE_AT_END_OF_LAYER_CHAR:     //if (rawChar==3)
				//requestToPauseAtEndOfLayer = 1; 
				break; //(when M790 is executed set flag to stop at end of this layer

			case AVAILABLE_4:	break; //if (rawChar==4)

			case SEND_STATUS_CHAR:    break; //if (rawChar==5)
				//M_Code_M775();break;// send live status on health of motion controller

			//case ASCII_ACK: //if (rawChar==6)
				//ShowNextDisplay(); break;
			case PING_REPLY:
				WorkingComPort->TxAcknowledgeCounter--; //keep track of how far behind we are
				if (WorkingComPort->TxAcknowledgeCounter < 0) WorkingComPort->TxAcknowledgeCounter = 0; //in case of underrun from reset condition
				if (WorkingComPort->pingSent)
				{
					ui_comm_add_char(0x06, UI_RECEIVE_COLOR);	
					WorkingComPort->pingSent = false;
				}
				else if (ui_comm_is_ack)
				{
					//ui_comm_add_event((const char*)&WorkRxChar, UI_RECEIVE_COLOR, ui_comm_is_hex);	
				}
				// if bleServer get the PING Replay from 407, send the PING_REPLY to client
				communication_add_char_to_ble_buffer(&bleDevice.TxBuffer, PING_REPLY);
				break;

			case PING_CHAR:     //if (rawChar==7)
				communication_add_char_to_serial_buffer(&WorkingComPort->TxBuffer, (uint8_t) PING_REPLY);
				WorkingComPort->TxAcknowledgeCounter = 0;
				WorkingComPort->RxAcknowledgeCounter = 0;
				
				// if bleClient get the PING from PC, send the PING to server
				if(systemconfig.serial2.mode == SERIAL2_MODE_BLE_MODEN)
					communication_add_char_to_ble_buffer(&bleDevice.TxBuffer, PING_CHAR);
				break;

			case ABORT_CHAR:  
				//				MotorX.PULSES_TO_GO = 0;
				//				MotorY.PULSES_TO_GO = 0;
				//				MotorZ.PULSES_TO_GO = 0;
								//requestToAbortAtEndOfMove = 1; break;  //if (rawChar==8)
									//this is a job abort, flush buffer NOW!!!!

			case URGENT_911_CMD_CHAR:     //if (rawChar==9)
				WorkingComPort->UrgentFlag = 1; //tell them this is a hot inject command line
				break;

			case CR_CHAR:
			case CMD_END_CHAR:  //if (rawChar==10) 0xA or 0xD  can trigger the end of line		
				if (WorkingComPort->CommandLineIdx <= 1)
				{
					//
					// if command length is 0, do nothing. 
					// for example, aaa 0xa 0xd, and WorkRxChar is CR_CHAR, command line' length should be 1
					// finally, WorkingComPort->CommandLineBuffer = '\0' or WorkingComPort->CommandLineBuffer = '\r'
					return;
				}
				WorkingComPort->RxAcknowledgeCounter++;
				WorkingComPort->CommandLineBuffer[WorkingComPort->CommandLineIdx] = WorkRxChar;
				parser_add_line_to_commandbuffer(WorkingComPort);
				WorkingComPort->UrgentFlag = 0; 
				WorkingComPort->CommandLineIdx = 0;				
				return;
			//case  13: 		return;//return char, just ignore
				
			case JOG_Z_TABLE_UP:    
				//JogMotorZFlag = 1; break;   //if (rawChar==11)

			case JOG_Z_TABLE_DOWN:  
				//JogMotorZFlag = -1; break;   //if (rawChar==12)

			case REPETREL_COMM_WATCHDOG_CHAR:   // (rawChar==14)
				//_repetrelCommWatchCount = REPETREL_COMM_WATCHDOG_START_VALUE;
				break;

			case JOG_DISPLAYplus: 
				//ShowNextDisplay(); 
				break;											// (rawChar==15)
			case JOG_DISPLAYminus:	
				//ShowPreviousDisplay(); 
				break;
			case VARIABLE_RESET:   
				// ClearSliceTimes(); ResetParseCounters(); 
				break;				//if (rawChar==17)
			}
		}
	}	
}

void communication_process_rx_ble(BleDevice* device)
{
	if (device->RxBuffer.Head == device->RxBuffer.Tail)return;//nothing to 
	ComBuffer* SourceBuff = &device->RxBuffer;
	uint8_t i = 0;
	char WorkRxChar;
	uint32_t idx = 0; 
	for (i = 0; i < PROCESS_MAX_CHARS_TO_READ_ON_ONE_SLICE; i++)
	{
		if (SourceBuff->Head == SourceBuff->Tail) break;
		//must be != because it is circular que.... can not simpley be less than
		WorkRxChar = SourceBuff->buffer[SourceBuff->Tail];
		SourceBuff->Tail++; //point to the next character
		SourceBuff->Tail &= (SourceBuff->Buffer_Size - 1); //circular que with even hex size....

		if (WorkRxChar  > 0x19 && WorkRxChar <= 0x7F)
		{
			//normal ascii character processing, below 20 hex are special control characters
			//communication_add_char_to_serial_buffer(TargetBuff, WorkRxChar);
			device->CommandLineBuffer[device->CommandLineIdx] = WorkRxChar;
			device->CommandLineIdx++;
			AddCharacterToAsciiArgs(device, WorkRxChar); //plug in the gcode arg
			if (idx >= 254)
			{
				device->CommandLineBuffer[254] = CMD_END_CHAR;
				parser_add_line_to_blebuffer(device);
				device->UrgentFlag = 0; 
				device->CommandLineIdx = 0;
			}
		}
		//if you get here, you must process special characters
		else
		{			
			switch (WorkRxChar)
			{
			case   0:                                       break; //null end of the string
			case TERMINATE_WAIT_CHAR:   		//if (rawChar==1)
				// Process_TERMINATE_WAIT_CHAR(); 
				break;

			case PAUSE_AT_END_OF_MOVE_CHAR:     //if (rawChar==2)
				//requestToPauseAtEndOfMove = 1; 
				break;//set flag to stop at end of this move

			case PAUSE_AT_END_OF_LAYER_CHAR:     //if (rawChar==3)
				//requestToPauseAtEndOfLayer = 1; 
				break; //(when M790 is executed set flag to stop at end of this layer

			case AVAILABLE_4:	break; //if (rawChar==4)

			case SEND_STATUS_CHAR:    break; //if (rawChar==5)
				//M_Code_M775();break;// send live status on health of motion controller

			//case ASCII_ACK: //if (rawChar==6)
				//ShowNextDisplay(); break;
			case PING_REPLY:
				if (run_mode == RUN_BLE_CLIENT && systemconfig.serial2.mode == SERIAL2_MODE_BLE_MODEN)
				{
					communication_add_char_to_serial_buffer(&ComUart2.TxBuffer, (uint8_t) PING_REPLY);
				}
				break;
			case PING_CHAR:     //if (rawChar==7)
				// if (run_mode == RUN_BLE_SERVER)
				// 	communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, PING_REPLY);
				communication_add_char_to_ble_buffer(&bleDevice.TxBuffer, PING_REPLY);
				break;
			case ABORT_CHAR:  
				break;
			case URGENT_911_CMD_CHAR:     //if (rawChar==9)
				device->UrgentFlag = 1; //tell them this is a hot inject command line
				break;
			case CR_CHAR:
			case CMD_END_CHAR:  //if (rawChar==10) 0xA or 0xD  can trigger the end of line		
				if (device->CommandLineIdx <= 1)
				{
					//
					// if command length is 0, do nothing. 
					// for example, aaa 0xa 0xd, and WorkRxChar is CR_CHAR, command line' length should be 1
					// finally, WorkingComPort->CommandLineBuffer = '\0' or WorkingComPort->CommandLineBuffer = '\r'
					return;
				}		
				device->CommandLineBuffer[device->CommandLineIdx] = WorkRxChar;
				parser_add_line_to_blebuffer(device);
				
				device->UrgentFlag = 0; 
				device->CommandLineIdx = 0;
				return;
			case TOGGLE_DIAG_DISPLAY:   
				if (run_mode == RUN_BLE_SERVER)
				{
					communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, WorkRxChar);
				}
				break;   
				//if toggle on-off sending the embedded lcd text to the serial display (rawChar==11)
			case DIAG_DISPLAY_CLEAR: 
				if (run_mode == RUN_BLE_SERVER)
				{
					communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, WorkRxChar);
				}
				break; //clear Diag LCD lines buffer.
			//case JOG_Z_TABLE_UP:    
				//JogMotorZFlag = 1; break;   //if (rawChar==11)

			//case JOG_Z_TABLE_DOWN:  
				//JogMotorZFlag = -1; break;   //if (rawChar==12)

			case REPETREL_COMM_WATCHDOG_CHAR:   // (rawChar==14)
				//_repetrelCommWatchCount = REPETREL_COMM_WATCHDOG_START_VALUE;
				break;

				
			case JOG_DISPLAYplus: 
				//ShowNextDisplay(); 
				if (run_mode == RUN_BLE_SERVER)
				{
					communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, WorkRxChar);
				}
				break;											// (rawChar==15)
			case JOG_DISPLAYminus:	
				//ShowPreviousDisplay(); 
				if (run_mode == RUN_BLE_SERVER)
				{
					communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, WorkRxChar);
				}
				break;
			case VARIABLE_RESET:   
				// ClearSliceTimes(); ResetParseCounters(); 
				break;				//if (rawChar==17)
			}
		}
	}	
}



void communication_process_tx_ble(BleDevice* device)
{
	uint8_t workcharacter;
	uint16_t numberOfXmitCharactersToSend = 0;
	if ((device->TxBuffer.Head != device->TxBuffer.Tail) || device->AcksWaiting)
	{
//		while (device->AcksWaiting)
//		{
//			//device->TxBuffer.command[numberOfXmitCharactersToSend] = ASCII_ACK; //stuff the acks in the front of the serial stream
//			device->AcksWaiting--;
//			numberOfXmitCharactersToSend++;
//		}
		while (device->TxBuffer.Head != device->TxBuffer.Tail)
		{ 
			workcharacter = device->TxBuffer.buffer[device->TxBuffer.Tail];
			device->TxBuffer.command[numberOfXmitCharactersToSend] = workcharacter;
			device->TxBuffer.Tail++;
			device->TxBuffer.Tail &= (device->TxBuffer.Buffer_Size - 1);
			numberOfXmitCharactersToSend++;
			if (workcharacter == CMD_END_CHAR || workcharacter == NULL_CHAR )break;//one line at a time please
			if (numberOfXmitCharactersToSend > CMD_MAX_SIZE) break;//limit the transmission packet size
		}
		if (numberOfXmitCharactersToSend > 0) {
			if (run_mode == RUN_BLE_SERVER)
				ble_server_send_data((uint8_t*)device->TxBuffer.command, numberOfXmitCharactersToSend);
			else if(run_mode == RUN_BLE_CLIENT)
			{
				ble_client_write_data((uint8_t*)device->TxBuffer.command, numberOfXmitCharactersToSend);
			}
		}
		return;//only process 1 message per tick
	}
}


void communication_process_tx_serial(COMPORT* device)
{
	if (device->RxAcknowledgeCounter > 0)
	{
		serial_uart_write_byte(device, ASCII_ACK);
		device->RxAcknowledgeCounter--;
		if (device->RxAcknowledgeCounter < 0) device->RxAcknowledgeCounter = 0;
	}
	if (device->TxBuffer.Head != device->TxBuffer.Tail)
	{
		serial_uart_write_byte(device, device->TxBuffer.buffer[device->TxBuffer.Tail]);
		device->TxBuffer.Tail++;
		device->TxBuffer.Tail &= (device->TxBuffer.Buffer_Size -1);
	}	
}
void communication_check_tx_uart1()
{
	communication_process_tx_serial(&ComUart1);
}
void communication_check_tx_uart2()
{
	communication_process_tx_serial(&ComUart2);
}
void communication_check_tx_ble()
{
	communication_process_tx_ble(&bleDevice);
}

void communication_check_rx_uart1()
{
	communication_process_rx_serial(&ComUart1);
}
void communication_check_rx_uart2()
{
	switch (systemconfig.serial2.mode) {
	case SERIAL2_MODE_SIMPLE:
		ParseIncommingLineToSimpleString(); // Simple parsing
		break;
	case SERIAL2_MODE_SECS:
		ParseIncommingLineToSecsString(); // Secs parsing
		break;
	case SERIAL2_MODE_BLE_MODEN:
		communication_process_rx_serial(&ComUart2);
		break;
	}
}
void communication_check_rx_ble()
{
	communication_process_rx_ble(&bleDevice);
}

void communication_check_tx()
{
	switch (tx_check_flag)
	{
	case 0: communication_process_tx_ble(&bleDevice); break;
	case 1: communication_process_tx_serial(&ComUart1); break;
	case 2: 
		communication_process_tx_serial(&ComUart2); break;
	}
	if (tx_check_flag >= 2) tx_check_flag = 0;
	else tx_check_flag++;
}

void communication_check_rx()
{
	switch (rx_check_flag)
	{
	case 0: communication_process_rx_ble(&bleDevice); break;
	case 1: communication_process_rx_serial(&ComUart1); break;
	case 2: 
		switch (systemconfig.serial2.mode)
		{
		case SERIAL2_MODE_SIMPLE:
			ParseIncommingLineToSimpleString(); // Simple parsing
			break;
		case SERIAL2_MODE_SECS:
			ParseIncommingLineToSecsString(); // Secs parsing
			break;
		case SERIAL2_MODE_BLE_MODEN:
			break;
		}
	}
	if (rx_check_flag >= 2) rx_check_flag = 0;
	else rx_check_flag++;
}


//send a commandline 
void communication_tx_commandline(COMPORT* comport, char* commandline)
{
	if (run_mode == RUN_BLE_CLIENT) return;
	if (comport->TxAcknowledgeCounter > 5) return;
	int len = strlen(commandline);
	communication_add_buffer_to_serial_buffer(&comport->TxBuffer, (uint8_t*)commandline, len); 
	comport->TxAcknowledgeCounter++;
	comport->NumberOfCharactersSent += len;
	if (comport->uart_id == UART_NUM_1)
	{
		ui_comm_add_log(commandline, UI_SEND_COLOR);	
	}
	else if (systemconfig.serial2.mode == SERIAL2_MODE_SIMPLE)
	{
		ui_simple_add_log(commandline, UI_SEND_COLOR);
	}
}

void SendPing()
{
	communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, PING_CHAR);
	MegComPort->pingSent = true;
	ui_comm_add_event((const char*)"Send Ping 0x7", UI_SEND_COLOR, false);
}

void SendDisplayStatusCode(bool isEnable)
{
	communication_add_char_to_serial_buffer(&MegComPort->TxBuffer, isEnable? 0x5: 0x4);
	ui_comm_add_event((const char*)isEnable ? "Serial Display enabled 0x5" : "Serial Display disabled 0x4", UI_SEND_COLOR, false);
}


void AddCharacterToAsciiArgs(BleDevice *WorkPort, uint8_t RawRxChar)
{
	//AddSerialCharToBuffer(&WorkPort->TxBuffer,RawRxChar); //echo the characters so we can diagnose the problem
	ComBuffer* SourceBuffer = &WorkPort->RxBuffer;//get the RxBuffer where the characters will come from.
	if (SourceBuffer->ReadyForAtof == 1) return;//process NOTHING if we do not have space....
//	while (SourceBuffer->Head != SourceBuffer->Tail)
//	{
//		RawRxChar = WorkBuff->buffer[WorkBuff->Tail]; //update the work character
//		WorkBuff->Tail++;
//		WorkBuff->Tail &= (WorkBuff->Buffer_Size - 1);

		if (SourceBuffer->CommentFlag>0)		//in comment mode, we should plug into a comment buffer for large transfers
		{//handle comments please
			if (RawRxChar == CMD_END_CHAR)
			{//purge till we see a cr, eol
				SourceBuffer->ReadyForAtof = 1;//we are ready to process the line, so flip up the convert to float flag
				*SourceBuffer->GCodeArgPtr++ = 0;//increment one more position in the character buffer and
				SourceBuffer->CommentFlag = 0;//stuff a NULL  (0) to indicate END OF STRING for that argument.
				WorkPort->RxAcknowledgeCounter++; //now you can also send an ACK back to who ever sent this nice command line
				ESP_LOGI(TAG, "Parsing command completed");
				return;
			}else 
			{
				*SourceBuffer->GCodeArgPtr++=RawRxChar;
				return;
			}
			//continue;//dont process comments for now
		}
		//debug only  AddSerialCharToBuffer(&WorkPort->TxBuffer, RawRxChar); //echo the characters so we can diagnose the problem
		switch (RawRxChar)
		{
		case CMD_END_CHAR:
			SourceBuffer->ReadyForAtof =1;
			*SourceBuffer->GCodeArgPtr++ = 0; //increment one more position in the character buffer and
			SourceBuffer->CommentFlag = 0; //stuff a NULL  (0) to indicate END OF 
			WorkPort->RxAcknowledgeCounter++;
			return;
			//this section is valid variable charcaters, that can be converted by Atof(string) function
		case '1': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '2': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '3': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '4': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '5': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '6': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '7': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '8': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '9': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '0': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '.': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '-': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		case '+': *SourceBuffer->GCodeArgPtr=RawRxChar;SourceBuffer->GCodeArgPtr++;*SourceBuffer->GCodeArgPtr=0; break;//load char, and set next as null
		//end of valid variable characters
		case ' ':   break;//*GCodeArgPtr=' ';GCodeArgPtr++;*GCodeArgPtr=0; break;//POINT TO THE NEXT POSITION PLEASE

		//this is the beginning of the KEY characters in gcode
		case 'A': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgA; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'B': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgB; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'C': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgC; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'D': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgD; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'E': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgE; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'F': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgF; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'G': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgG; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'H': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgH; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'I': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgI; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'J': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgJ; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'K': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgK; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'L': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgL; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'M': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgM; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'N': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgN; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'O': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgO; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'P': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgP; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'Q': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgQ; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'R': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgR; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'S': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgS; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'T': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgT; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'U': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgU; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'V': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgV; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'W': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgW; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'X': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgX; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'Y': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgY; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case 'Z': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgZ; *SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		//

		case '*': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgSplat;*SourceBuffer->GCodeArgPtr++=RawRxChar;SourceBuffer->ArgumentLength=0; return;
		case '/':
		case ';': SourceBuffer->GCodeArgPtr = (char*)&SourceBuffer->AsciiArgs.GCodeArgComment; SourceBuffer->CommentFlag = 1; SourceBuffer->ArgumentLength=0; return;
//					commentFlag=1;*WorkBuff->GCodeArgPtr++=RawRxChar;WorkBuff->ArgumentLength=0; return;
		}
		SourceBuffer->ArgumentLength++;
		if (SourceBuffer->ArgumentLength >= (!SourceBuffer->CommentFlag ? MAX_CHARS_FOR_PARAMETER : COMMENT_STRING_LENGTH))  // -1 because need room for the NULL
		{
			// ProcessingError=1;
			return;//just dont do anything yet
		}
	//}
}
