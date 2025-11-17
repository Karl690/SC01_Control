#include <driver/pulse_cnt.h>
#include <driver/gpio.h>
#include "pcnt.h"

PCNT_INFO pcnt_info = { 0, 0, 0, 0, 0, 0 };

#define MAX_ADC12 						4096
#define TEMP_FRAC_BITS                  5
#define TEMP_SCALE                      32
#define TEMP_SCALEF                     32.0f
#define MAX_TEMP                        0x7fff  // max positive

pcnt_unit_handle_t PulseCounter_1 = NULL;
pcnt_unit_handle_t PulseCounter_2 = NULL;
int PwmTimerReloadRegister = 0;
int TemperatureFreq;
int Battery_V_Freq;
float RtdVoltage;
float Temperature;
float BatteryVoltage;
PcntTableStruct const RtdTable_1K[] __attribute__((aligned(4))) =
{
	// 1K RTD -- based on datasheet
		// 20 entries; 4 bytes each; 80 bytes total
		// (uint16_t)adcVal, (uint16_t)(tempC * 32)    (MUST be in order with increasing adcVals)
	{ 0, (-242 << TEMP_FRAC_BITS) },
	// extrapolated to get -242 as 0 ohms (yielding 0 adc value with 2K divider)
	{ 347, (-200 << TEMP_FRAC_BITS) },
	// 1K mfg table starts at -200
	{ 678, (-150 << TEMP_FRAC_BITS) },
	{ 948, (-100 << TEMP_FRAC_BITS) },
	{ 1173, (-50 << TEMP_FRAC_BITS) },
	{ 1365, (0 << TEMP_FRAC_BITS) },
	{ 1531, (50 << TEMP_FRAC_BITS) },
	{ 1676, (100 << TEMP_FRAC_BITS) },
	{ 1803, (150 << TEMP_FRAC_BITS) },
	{ 1916, (200 << TEMP_FRAC_BITS) },
	{ 2017, (250 << TEMP_FRAC_BITS) },
	{ 2107, (300 << TEMP_FRAC_BITS) },
	{ 2189, (350 << TEMP_FRAC_BITS) },
	{ 2263, (400 << TEMP_FRAC_BITS) },
	{ 2330, (450 << TEMP_FRAC_BITS) },
	{ 2392, (500 << TEMP_FRAC_BITS) },
	{ 2448, (550 << TEMP_FRAC_BITS) },
	{ 2500, (600 << TEMP_FRAC_BITS) },
	// 1K mfg tables ends at 600
	{ 2809, (999 << TEMP_FRAC_BITS) },
	// extrapolated to get 999 as 4370 ohms (yielding 2809 adc value with 2K divider)
	{ MAX_ADC12, MAX_TEMP },
	// in reality, to get to MAX_ADC, would take about 10 billion degrees.
};


void pcnt_init(void) {
	//conigure the pulse counter  pcnt
	pcnt_unit_config_t cfg0 = { 
		.low_limit = -1,
		.high_limit = 32000, 
	};
	cfg0.flags.accum_count = true; //keep counting, even after reading
	ESP_ERROR_CHECK(pcnt_new_unit(&cfg0, &PulseCounter_1)); //set up counter1

	pcnt_chan_config_t Channel_configuration = { 
		.edge_gpio_num = Counter1_INPUT_PIN, //assign input pin for pulse counter
		.level_gpio_num = -1, // Level signal not used for simple pulse counting
		//do not invert or set defult level, count on rising edge
		.flags = {
		.invert_edge_input = false,
		.invert_level_input = false, 
	}
	};
	pcnt_channel_handle_t chan0;
	ESP_ERROR_CHECK(pcnt_new_channel(PulseCounter_1, &Channel_configuration, &chan0));
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan0, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
	ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan0, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));
	//now setup counter 2
	ESP_ERROR_CHECK(pcnt_new_unit(&cfg0, &PulseCounter_2)); //set up counter2
	Channel_configuration.edge_gpio_num = Counter2_INPUT_PIN; //assign the gpio pin to counter2
	//
	pcnt_channel_handle_t chan1;
	ESP_ERROR_CHECK(pcnt_new_channel(PulseCounter_2, &Channel_configuration, &chan1));
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan1, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
	ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan1, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

	pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 100 }; //set it to 100ns as we are using 1mhz input pulse
	ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(PulseCounter_1, &filter_config));
	ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(PulseCounter_2, &filter_config));

	ESP_ERROR_CHECK(pcnt_unit_enable(PulseCounter_1));
	ESP_ERROR_CHECK(pcnt_unit_enable(PulseCounter_2));

	ESP_ERROR_CHECK(pcnt_unit_clear_count(PulseCounter_1));
	ESP_ERROR_CHECK(pcnt_unit_clear_count(PulseCounter_2));
   
	ESP_ERROR_CHECK(pcnt_unit_start(PulseCounter_1));
	ESP_ERROR_CHECK(pcnt_unit_start(PulseCounter_2));  
}

void EnableCounter() {
	ESP_ERROR_CHECK(pcnt_unit_start(PulseCounter_1));
	ESP_ERROR_CHECK(pcnt_unit_start(PulseCounter_2));
}


void DisableCounter() {
	ESP_ERROR_CHECK(pcnt_unit_stop(PulseCounter_1));
	ESP_ERROR_CHECK(pcnt_unit_stop(PulseCounter_2));
}

// calculate the real data every 1s.
void Calculate_Heater_DutyCycle() {
	//at this point the counter has been read and converted to a voltage "RtdVoltage"
   // RtdVoltage = TemperatureFreq * systemconfig.pcnt.rtd_scale; old method
//    Temperature = Convert_using_Lookup_Table(RtdTable_1K, RtdVoltage);//use lookup table to convert voltage to temperature
//    pcnt_info.temperature = Temperature;
	//pcnt_info.duty++;
//	pcnt_info.duty++; //change by 1 each second
//	if (pcnt_info.duty > 16)pcnt_info.duty = 0;
//	return;
	if (systemconfig.pcnt.enabled)
	{
		float deltaTemp = systemconfig.pcnt.programmed_temperature - pcnt_info.temperature;
		if (deltaTemp < 0)deltaTemp = 0;
		pcnt_info.duty = (int)deltaTemp;
		return;
	}
	pcnt_info.duty = 0;
}

void SetPwmOutput()
{
	
	//test pulse 
	systemconfig.pcnt.duty_test++; //count up
	PwmTimerReloadRegister++;
	if (PwmTimerReloadRegister > 32)PwmTimerReloadRegister = 0;
	//if (pcnt_info.duty > PwmTimerReloadRegister)
	if (systemconfig.pcnt.duty_test & 0x0001)
	{
		gpio_set_level(ControlOutput_PIN, 1); //disable the heater until code is stable
	}
	else
	{
		gpio_set_level(ControlOutput_PIN, 0); //disable the heater until code is stable	
	}
	//	if (systemconfig.pcnt.duty_test == 0)
	//	{
	//		//normal duty cycle process
	//		PwmTimerReloadRegister = pcnt_info.duty; //setthe reload timer		
	//	}
	//	else
	//	{//first do a range check , duty must be between 0 and 100%
	//		if (systemconfig.pcnt.duty_test > 100)systemconfig.pcnt.duty_test = 100;
	//		if (systemconfig.pcnt.duty_test <0)systemconfig.pcnt.duty_test = 0;
	//		PwmTimerReloadRegister = systemconfig.pcnt.duty_test;
	//	}
}

void Scale_BatteryVoltage() {
	BatteryVoltage = Battery_V_Freq * systemconfig.pcnt.battery_scale;
	pcnt_info.bat_volt = BatteryVoltage;

     
}

float convertRtdDataFromRawADCValue(const PcntTableStruct* adcTable, float RTD_Voltage) 
{
	uint16_t Index = 0; //need a pointer to know where in the table we are working
	int tablesize = sizeof adcTable;
	float conversionValue = 0;
	if (RTD_Voltage == 0)return conversionValue;//IF raw is 0 then we are at 0
	//need check for max a2d value also, to prevent bogus information.
	//now we will walk thru the table to find the index to the first RAW data that is >= rawData
	for (Index = 0; Index < 32; Index++)  //table size did not work, it gives the 4 bytes of the entry
	{
		if (adcTable[Index].adcRaw >= RTD_Voltage) break;//found our index
	}
	//check for 3 unique cases
	//if rawdata and table.rawdata match, then the temperature also matches
	//if rawdata is 0
	//if rawdata is == max_adc12 value
	//if any  of these are true,then we simply return the adctable.value
	if ((adcTable[Index].adcRaw == RTD_Voltage) || (RTD_Voltage == 0) || (adcTable[Index].adcRaw >= MAX_ADC12))
	{
		//lucky, we hit the value exaclty, so we are actually exaclty the table entry in size
		conversionValue = (float)adcTable[Index].value;
		return conversionValue;
	}
	//ok at this point were have a index to the first RAW value that is greater than the actual raw
	//dat a value, we need to make sure our rawdata value falls between the 2 points in the table
	if (Index > 0) Index--; //if we are already at zero, we will use this pointer, 
	float DeltaTemperatureValue = adcTable[Index + 1].value - adcTable[Index].value;
	float DeltaRawData = adcTable[Index + 1].adcRaw - adcTable[Index].adcRaw;
	float conversionCoeffecient = DeltaTemperatureValue / DeltaRawData;
	//now we have the coeffecient between the 2 points in the table
	//next we will get the offset value between the first point and the raw data
	int rawRemainder = RTD_Voltage - adcTable[Index].adcRaw;
	float offsetTemperatureValue = rawRemainder*conversionCoeffecient;
	conversionValue = offsetTemperatureValue + adcTable[Index].value;
	conversionValue /= TEMP_SCALEF; //divide by the temp bits to scale to degrees c.
	return conversionValue;
}

void Read_Counters() {
	//reads both counter1 and 2, then resets counters to 0, used in 100hz loop
	//so we are actually getting frequency in 100 hz resolution
	ESP_ERROR_CHECK(pcnt_unit_get_count(PulseCounter_1, &pcnt_info.count01));
	ESP_ERROR_CHECK(pcnt_unit_clear_count(PulseCounter_1));


	ESP_ERROR_CHECK(pcnt_unit_get_count(PulseCounter_2, &pcnt_info.count02));
	ESP_ERROR_CHECK(pcnt_unit_clear_count(PulseCounter_2));

	//now process the variables into voltage
	//the AD7740 V/f chip uses 100khz as zero volt base line
	//since we are sampling at 100hz, we should subtract 1000 from count to get actual voltage
	if (pcnt_info.count02 < 1000)//adjust for 0 volt freq offset of 1000
	{Battery_V_Freq = 0; }
	else
	{Battery_V_Freq = pcnt_info.count02 - 1000; }
	//adjust the temperature next
	if (pcnt_info.count01 < 1000)//adjust for 0 volt freq offset of 1000
	{TemperatureFreq = 0; }
	else
	{TemperatureFreq = pcnt_info.count01 - 1000; }
	// next we can apply the scaling logic to the net count
	//the V/F internal reference is 2.5V so 2.5V should have 90% 
	//or 900khz for the full scale frequency, that would correspond to a
	//2.5*900000/100   or   2.5v=8000  >>  or   voltage=NetCount/3200 
	
	RtdVoltage = (float)((float)TemperatureFreq / 3200); //systemconfig.pcnt.rtd_scale;
	pcnt_info.rtd_volt = RtdVoltage; //update global
	//battery voltage next
	BatteryVoltage = (float)((float)Battery_V_Freq / 526); //systemconfig.pcnt.battery_scale;
	pcnt_info.bat_volt = BatteryVoltage; //update global variable
	//convert to temperature
	Temperature = convertRtdDataFromRawADCValue(RtdTable_1K, (RtdVoltage*systemconfig.pcnt.rtd_scale)); //use lookup table to convert voltage to temperature
	pcnt_info.temperature = Temperature;
	
}
