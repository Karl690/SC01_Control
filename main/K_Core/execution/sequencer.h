#pragma once
#include "main.h"

#define SIZE_OF_COMMAND_QUEUE 	10
#define MAX_COMMAND_LEN 		50


#define UNPROCESSED -1 // used to init cmdLink on cmdQue CleanUpPointers ARG_N_PRESENT

#define MAX_CHARS_FOR_PARAMETER 	20
#define COMMENT_STRING_LENGTH   	(256+3)	//+3 for leading and trailing delim + NULL_CHAR
#define GCODE_STRING_ARG_LENGTH 	(256+3)	//+3 for leading and trailing delim + NULL_CHAR

typedef struct {
	unsigned A : 1;
	unsigned B : 1;
	unsigned C : 1;
	unsigned D : 1;
	unsigned E : 1;
	unsigned F : 1;
	unsigned G : 1;
	unsigned H : 1;
	unsigned I : 1;
	unsigned J : 1;
	unsigned K : 1;
	unsigned L : 1;
	unsigned M : 1;
	unsigned N : 1;
	unsigned O : 1;
	unsigned P : 1;
	unsigned Q : 1;
	unsigned R : 1;
	unsigned S : 1;
	unsigned T : 1;
	unsigned U : 1;
	unsigned V : 1;
	unsigned W : 1;
	unsigned X : 1;
	unsigned Y : 1;
	unsigned Z : 1;
} flag_t;
typedef enum {
	//NOTE: items 2 and 3 behave the same, but commands should be defined as one or the other for possible future enhancements.
	UNDEFINED,
	SINGLE_STEP,  // single step .... wait for empty queue, set ForceQToEmpty, process, reset ForceQToEmpty
	ADD_TO_MQ,  // add to Q or process in order with Q (tag as processed if not added to Q, ie G90)
	IMPACTS_DESTINATION_CALC,  // ONLY affects position (ie, change of offsets) or has no impact on position/motion and does not need to be synced
	SYNCS_WITH_MOTION,   // skip, but tag, until prior motion complete...unless Q is empty, the just process
	
} command_t;

typedef struct {
	float A;    //0 motor A, 4th axis
	float B;    //1 motor B, 4th axis
	float C;    //2 motor C, 4th axis
	float D;    //3 tool working diameter offset
	float E;    //4 used for extrusion value
	float F;	//5
	float G;    //6 gcode
	float H;    //7 Tool working length offset
	float I;    //8 used for circular arc commands, defines offset to center of circles
	float J;    //9 used for circular arc commands, defines offset to center of circle
	float K;	//10
	float L;	//11
	float M;    //12 mcode
	float N;	//13
	float O;	//14
	float P;	//15
	float Q;	//16
	float R;	//17
	float S;	//18
	float T;    // tool number
	float U;	//19
	float V;	//20
	float W;	//21
	float X;    //22 motor X, 1st axis
	float Y;    //23 motor Y, 2nd axis
	float Z;    //24 motor Z, 3rd axis
	float CS;	//25
	uint8_t Comment[COMMENT_STRING_LENGTH];
	//union {
	//   flag_t flag;
	//   unsigned int flags;
	//} valid;
	command_t cmdType;  // defined type indicating how this type of command interracts with the motionQ command_t
	int cmdLink;        // index to parent command (link to motionQ from deferredQ) toolNumber	
} GMCommandStructure;

extern GMCommandStructure cmdQue[SIZE_OF_COMMAND_QUEUE];
extern GMCommandStructure *ExecutionPtr;

#define INVALID_ARG_VALUE (0xFFFF) // replace extern float Invalid;

#define ARG_A   (ExecutionPtr->A)
#define ARG_B   (ExecutionPtr->B)
#define ARG_C   (ExecutionPtr->C)
#define ARG_D   (ExecutionPtr->D)
#define ARG_E   (ExecutionPtr->E)
#define ARG_F   (ExecutionPtr->F)
#define ARG_G   (ExecutionPtr->G)
#define ARG_H   (ExecutionPtr->H)
#define ARG_I   (ExecutionPtr->I)
#define ARG_J   (ExecutionPtr->J)
#define ARG_K   (ExecutionPtr->K)
#define ARG_L   (ExecutionPtr->L)
#define ARG_M   (ExecutionPtr->M)
#define ARG_N   (ExecutionPtr->N)
#define ARG_O   (ExecutionPtr->O)
#define ARG_P   (ExecutionPtr->P)
#define ARG_Q   (ExecutionPtr->Q)
#define ARG_R   (ExecutionPtr->R)
#define ARG_S   (ExecutionPtr->S)
#define ARG_T   (ExecutionPtr->T)
#define ARG_U   (ExecutionPtr->U)
#define ARG_V   (ExecutionPtr->V)
#define ARG_W   (ExecutionPtr->W)
#define ARG_X   (ExecutionPtr->X)
#define ARG_Y   (ExecutionPtr->Y)
#define ARG_Z   (ExecutionPtr->Z)
#define ARG_CS  (ExecutionPtr->CS)
#define ARG_COMMNET  (ExecutionPtr->Comment)

#define ARG_A_PRESENT   (ARG_A != INVALID_ARG_VALUE)
#define ARG_B_PRESENT   (ARG_B != INVALID_ARG_VALUE)
#define ARG_C_PRESENT   (ARG_C != INVALID_ARG_VALUE)
#define ARG_D_PRESENT   (ARG_D != INVALID_ARG_VALUE)
#define ARG_E_PRESENT   (ARG_E != INVALID_ARG_VALUE)
#define ARG_F_PRESENT   (ARG_F != INVALID_ARG_VALUE)
#define ARG_G_PRESENT   (ARG_G != INVALID_ARG_VALUE)
#define ARG_H_PRESENT   (ARG_H != INVALID_ARG_VALUE)
#define ARG_I_PRESENT   (ARG_I != INVALID_ARG_VALUE)
#define ARG_J_PRESENT   (ARG_J != INVALID_ARG_VALUE)
#define ARG_K_PRESENT   (ARG_K != INVALID_ARG_VALUE)
#define ARG_L_PRESENT   (ARG_L != INVALID_ARG_VALUE)
#define ARG_M_PRESENT   (ARG_M != INVALID_ARG_VALUE)
#define ARG_N_PRESENT   (ARG_N != INVALID_ARG_VALUE)
#define ARG_O_PRESENT   (ARG_O != INVALID_ARG_VALUE)
#define ARG_P_PRESENT   (ARG_P != INVALID_ARG_VALUE)
#define ARG_Q_PRESENT   (ARG_Q != INVALID_ARG_VALUE)
#define ARG_R_PRESENT   (ARG_R != INVALID_ARG_VALUE)
#define ARG_S_PRESENT   (ARG_S != INVALID_ARG_VALUE)
#define ARG_T_PRESENT   (ARG_T != INVALID_ARG_VALUE)
#define ARG_U_PRESENT   (ARG_U != INVALID_ARG_VALUE)
#define ARG_V_PRESENT   (ARG_V != INVALID_ARG_VALUE)
#define ARG_W_PRESENT   (ARG_W != INVALID_ARG_VALUE)
#define ARG_X_PRESENT   (ARG_X != INVALID_ARG_VALUE)
#define ARG_Y_PRESENT   (ARG_Y != INVALID_ARG_VALUE)
#define ARG_Z_PRESENT   (ARG_Z != INVALID_ARG_VALUE)

#define ARG_A_MISSING   (ARG_A == INVALID_ARG_VALUE)
#define ARG_B_MISSING   (ARG_B == INVALID_ARG_VALUE)
#define ARG_C_MISSING   (ARG_C == INVALID_ARG_VALUE)
#define ARG_D_MISSING   (ARG_D == INVALID_ARG_VALUE)
#define ARG_E_MISSING   (ARG_E == INVALID_ARG_VALUE)
#define ARG_F_MISSING   (ARG_F == INVALID_ARG_VALUE)
#define ARG_G_MISSING   (ARG_G == INVALID_ARG_VALUE)
#define ARG_H_MISSING   (ARG_H == INVALID_ARG_VALUE)
#define ARG_I_MISSING   (ARG_I == INVALID_ARG_VALUE)
#define ARG_J_MISSING   (ARG_J == INVALID_ARG_VALUE)
#define ARG_K_MISSING   (ARG_K == INVALID_ARG_VALUE)
#define ARG_L_MISSING   (ARG_L == INVALID_ARG_VALUE)
#define ARG_M_MISSING   (ARG_M == INVALID_ARG_VALUE)
#define ARG_N_MISSING   (ARG_N == INVALID_ARG_VALUE)
#define ARG_O_MISSING   (ARG_O == INVALID_ARG_VALUE)
#define ARG_P_MISSING   (ARG_P == INVALID_ARG_VALUE)
#define ARG_Q_MISSING   (ARG_Q == INVALID_ARG_VALUE)
#define ARG_R_MISSING   (ARG_R == INVALID_ARG_VALUE)
#define ARG_S_MISSING   (ARG_S == INVALID_ARG_VALUE)
#define ARG_T_MISSING   (ARG_T == INVALID_ARG_VALUE)
#define ARG_U_MISSING   (ARG_U == INVALID_ARG_VALUE)
#define ARG_V_MISSING   (ARG_V == INVALID_ARG_VALUE)
#define ARG_W_MISSING   (ARG_W == INVALID_ARG_VALUE)
#define ARG_X_MISSING   (ARG_X == INVALID_ARG_VALUE)
#define ARG_Y_MISSING   (ARG_Y == INVALID_ARG_VALUE)
#define ARG_Z_MISSING   (ARG_Z == INVALID_ARG_VALUE)

void Sequencer();