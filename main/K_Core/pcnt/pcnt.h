#pragma once
#include "main.h"


#define SAMPLE_TIME_MS      1000

#define PCNT_BATTERY_SCAL_VALUE  3731.0
#define PCNT_RTD_SCAL_VALUE      1270

#define PCNT_TEMP_SCAL_VALUE       1.00
#define PCNT_DUTY_SCAL_VALUE       1.0

#define PCNT_BATTERY_EMPTY 				6.0
#define PCNT_BATTERY_FULL				9.0

#define ADC_NUM_SAMPLES                 10  // 10 values saved; toss high and low to get average
#define ADC_SHIFT_FOR_AVG               3
 
typedef struct 
{
	int count01;
	int count02;
	float temperature; //'C    
	int16_t freq; //in Hz
	int duty; //in %
	float rtd_volt; //in V
	float bat_volt; //in V
	float bat_percent;//battery percent
} PCNT_INFO;

typedef struct {
	uint16_t    adcRaw; // expected adcValue for specified temp
	int16_t      value; // if temperature (s10.5 format - 1/32 degree) at specified adcValue; position, just value, etc
} PcntTableStruct;

extern uint16_t SmoothSampleIndex;
extern PCNT_INFO pcnt_info;
extern int PwmTimerReloadRegister;
extern float bat_percent;
extern float Kfactor;
extern uint16_t     RTDsampleHistory[ADC_NUM_SAMPLES + 4]; // last N reads from ADC
extern uint16_t     BatterysampleHistory[ADC_NUM_SAMPLES + 4]; // last N reads from ADC
void pcnt_init(void);
void EnableCounter();
void DisableCounter();
void Calculate_Heater_DutyCycle();
void Scale_BatteryVoltage();
//float pcnt_convert_temperature(float voltage);
float convertRtdDataFromRawADCValue(const PcntTableStruct* adcTable, float voltage);
void SetPwmOutput();
void Read_Counters();
