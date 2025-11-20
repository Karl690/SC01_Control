#include "parser.h"
#include "RevisionHistory.h"
#include "L_Core/bluetooth/ble.h"
#include "L_Core/ui/ui-comm.h"
#include "K_Core/execution/cmdprocessor.h"
#include "K_Core/communication/communication.h"
#include "K_Core/execution/sequencer.h"

uint32_t ParsedLineCounter = 0;
uint32_t RxAcknowledgeCounter = 0;
uint32_t ProcessingError = 0;
uint32_t gcodeLineNumber = 0;
uint32_t GCodeArgsReadyToConvert = 0;

char parse_buffer[256] = { 0 };
void parser_add_line_to_commandbuffer(COMPORT* workingComport)
{
	if (workingComport->UrgentFlag)
	{
		strcpy(cmd_CommandsInQueBuffer[0], workingComport->CommandLineBuffer); // copy the command string to Que buffer
	}
	else
	{
		strcpy(cmd_CommandsInQueBuffer[cmd_NextCommandInsertionPointer], workingComport->CommandLineBuffer); // copy the command string to Que buffer
		if (run_mode == RUN_BLE_SERVER && systemconfig.serial2.mode == SERIAL2_MODE_BLE_MODEN)
		{
			//send line to server 
			communication_add_string_to_ble_buffer(&bleDevice.TxBuffer, cmd_CommandsInQueBuffer[cmd_NextCommandInsertionPointer]);
			communication_add_char_to_ble_buffer(&bleDevice.TxBuffer, '\n');
		}
		cmd_NextCommandInsertionPointer++;
		
		if (cmd_NextCommandInsertionPointer >= SIZE_OF_COMMAND_QUEUE) cmd_NextCommandInsertionPointer = 1;
		memset(workingComport->CommandLineBuffer, 0, 255);
	}
	workingComport->RxBuffer.ReadyForAtof = 1;
	workingComport->RxAcknowledgeCounter++;
	memset(workingComport->CommandLineBuffer, 0, 256); //reset commandlinebuffer
	cmd_CommandsInQue++;
}

void parser_add_line_to_blebuffer(BleDevice* workingComport)
{
	if (workingComport->UrgentFlag)
	{
		strcpy(cmd_CommandsInQueBuffer[0], workingComport->CommandLineBuffer); // copy the command string to Que buffer
	}
	else
	{
		strcpy(cmd_CommandsInQueBuffer[cmd_NextCommandInsertionPointer], workingComport->CommandLineBuffer); // copy the command string to Que buffer
		if (run_mode == RUN_BLE_CLIENT && systemconfig.serial2.mode == SERIAL2_MODE_BLE_MODEN)
		{
			// send the data from Ble server to Uart2.
			communication_add_string_to_serial_buffer(&ComUart2.TxBuffer, cmd_CommandsInQueBuffer[cmd_NextCommandInsertionPointer]);
			communication_add_char_to_serial_buffer(&ComUart2.TxBuffer, '\n');
		}
		cmd_NextCommandInsertionPointer++;
		if (cmd_NextCommandInsertionPointer >= SIZE_OF_COMMAND_QUEUE) cmd_NextCommandInsertionPointer = 1;
		memset(workingComport->CommandLineBuffer, 0, 255);
	}
	workingComport->RxBuffer.ReadyForAtof = 1;
	RxAcknowledgeCounter++;
	memset(workingComport->CommandLineBuffer, 0, 256); //reset commandlinebuffer
	cmd_CommandsInQue++;
}

void InvalidateAllCmdArgs(GMCommandStructure *cmdPtr)
{
	cmdPtr->A = INVALID_ARG_VALUE;
	cmdPtr->B = INVALID_ARG_VALUE;
	cmdPtr->C = INVALID_ARG_VALUE;
	cmdPtr->D = INVALID_ARG_VALUE;
	cmdPtr->E = INVALID_ARG_VALUE;
	cmdPtr->F = INVALID_ARG_VALUE;
	cmdPtr->G = INVALID_ARG_VALUE;
	cmdPtr->H = INVALID_ARG_VALUE;
	cmdPtr->I = INVALID_ARG_VALUE;
	cmdPtr->J = INVALID_ARG_VALUE;
	cmdPtr->K = INVALID_ARG_VALUE;
	cmdPtr->L = INVALID_ARG_VALUE;
	cmdPtr->M = INVALID_ARG_VALUE;
	cmdPtr->N = INVALID_ARG_VALUE;
	cmdPtr->O = INVALID_ARG_VALUE;
	cmdPtr->P = INVALID_ARG_VALUE;
	cmdPtr->Q = INVALID_ARG_VALUE;
	cmdPtr->R = INVALID_ARG_VALUE;
	cmdPtr->S = INVALID_ARG_VALUE;
	cmdPtr->T = INVALID_ARG_VALUE;
	cmdPtr->U = INVALID_ARG_VALUE;
	cmdPtr->V = INVALID_ARG_VALUE;
	cmdPtr->W = INVALID_ARG_VALUE;
	cmdPtr->X = INVALID_ARG_VALUE;
	cmdPtr->Y = INVALID_ARG_VALUE;
	cmdPtr->Z = INVALID_ARG_VALUE;
	cmdPtr->CS = INVALID_ARG_VALUE;
}

void ResetGcodeParseBuffer(ComBuffer* BufferToReset)
{
	//this will reset the ascii input buffers by putting a null in the first character
	BufferToReset->AsciiArgs.GCodeArgComment[0] = 0; //set comment to null as well
	BufferToReset->AsciiArgs.GCodeArgComment[1] = 0; //many routines look past the SOAPSTRING_CHAR location for the comment string.
	BufferToReset->AsciiArgs.GCodeArgA[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgB[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgC[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgD[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgE[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgF[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgG[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgH[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgI[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgJ[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgK[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgL[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgM[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgN[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgO[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgP[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgQ[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgR[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgS[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgT[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgU[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgV[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgW[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgX[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgY[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgZ[0] = 0;
	BufferToReset->AsciiArgs.GCodeArgSplat[0] = 0;
	BufferToReset->GCodeArgPtr = (char*)&BufferToReset->AsciiArgs.GCodeArgComment; //default first character
}

void ConvertArgs2Float(ComBuffer* WorkBuff) 
{
	//at this point we have already read the charcters and put them in each key arg buffer so we can convert to float
//	if (CommandsInQue >= (SIZE_OF_COMMAND_QUEUE - 3)) return;//dont take more commands unless you can handle them
	ExecutionPtr=&cmdQue[NextCommandInsertionPointer];
	 //point to the input parse buffer
	InvalidateAllCmdArgs(ExecutionPtr);//clear old variables please

	// ComBuffer* WorkBuff = &WorkPort->RxBuffer;

	//WorkBuff->RxLineCount --; //WorkComm->RxBuffer.RxLineCount--;//drop down a command please
	//ComBuffer* WorkBuff = WorkComm->UrgentFlag? &WorkComm->RxUrgentBuffer: &WorkComm->RxBuffer;
//	--------------------------------------------------------------------------------------
	//now the execution pointer is pointing to the correct base address in the cmdque
	//so we can directly convert to float from the ascii buffer to the correct cmdque location
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgA, &ExecutionPtr->A);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgB, &ExecutionPtr->B);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgC, &ExecutionPtr->C);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgD, &ExecutionPtr->D);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgE, &ExecutionPtr->E);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgF, &ExecutionPtr->F);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgG, &ExecutionPtr->G);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgH, &ExecutionPtr->H);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgI, &ExecutionPtr->I);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgJ, &ExecutionPtr->J);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgK, &ExecutionPtr->K);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgL, &ExecutionPtr->L);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgM, &ExecutionPtr->M);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgN, &ExecutionPtr->N);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgO, &ExecutionPtr->O);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgP, &ExecutionPtr->P);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgQ, &ExecutionPtr->Q);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgR, &ExecutionPtr->R);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgS, &ExecutionPtr->S);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgT, &ExecutionPtr->T);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgU, &ExecutionPtr->U);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgV, &ExecutionPtr->V);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgW, &ExecutionPtr->W);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgX, &ExecutionPtr->X);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgY, &ExecutionPtr->Y);
	processArgs((char*)&WorkBuff->AsciiArgs.GCodeArgZ, &ExecutionPtr->Z);
	strcpy((char*)&ExecutionPtr->Comment[0], WorkBuff->AsciiArgs.GCodeArgComment);
	ExecutionPtr->cmdType = UNDEFINED;
	ExecutionPtr->cmdLink = UNPROCESSED;

	ARG_N = ++gcodeLineNumber;//update the gcodelinenumber for error and debug syncronization
	if (ARG_N_PRESENT)gcodeLineNumber = ARG_N;//update to new line number if gcode has  N###  as argument

	GCodeArgsReadyToConvert++; //signal that the line is parsed and ready to add to actionQue
	
	  NextCommandInsertionPointer++;
	  if (NextCommandInsertionPointer >=(SIZE_OF_COMMAND_QUEUE))  NextCommandInsertionPointer=1;
	  CommandsInQue++;//increment the commands on the que to do

	WorkBuff->ReadyForAtof=0;//enable more character processing of the next incoming command line
	ParsedLineCounter++;//diagnostic counting only
	ResetGcodeParseBuffer(WorkBuff);      //clear first character of each argument and get ready for next char
	RxAcknowledgeCounter ++;//telluart to send ack to whoever sent this command line
}

void ProcessAsciiArgs2()
{
	if (bleDevice.RxBuffer.ReadyForAtof)
	{
		ConvertArgs2Float(&bleDevice.RxBuffer); //convert from ascii to digital arguments
		return;
	}
}
