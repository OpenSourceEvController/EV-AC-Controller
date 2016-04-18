// FOR Hall Effect Throttle.
#include "ACController.h"

/*****************Config bit settings****************/
_FOSC(0xFFFF & XT_PLL16);//XT_PLL4); // Use XT with external crystal from 4MHz to 10MHz.  FRM Pg. 178
// nominal clock is 128kHz.  The counter is 1 byte. 
#define DEBUG
//#define SPREAD_SPECTRUM_SWITCHING
//#define STANDARD_THROTTLE
#ifdef DEBUG
	_FWDT(WDT_OFF);
#else
	_FWDT(WDT_ON & WDTPSA_64 & WDTPSB_8); // See Pg. 709 in F.R.M.  Timeout in 1 second or so.  128000 / 64 / 8 / 256
#endif
 
_FBORPOR(0xFFFF & BORV_20 & PWRT_64 & MCLR_EN & PWMxL_ACT_HI & PWMxH_ACT_HI); // Brown Out voltage set to 2v.  Power up time of 64 ms. MCLR is enabled. 
// PWMxL_ACT_HI means PDC1 = 0 would mean PWM1L is 0 volts.  Just the opposite of how I always thought it worked.  haha.
// PWMxH_ACT_HI means, in complementary mode, that PDC1 = 0 would mean PWM1H is 5v, because it would be "active" (the complement of PWM1L), and thus HIGH. 
_FGS(CODE_PROT_OFF);  

#define Fcy 30000000L


SavedValuesStruct savedValuesDefault = {
	16,			// PI loop P gain for 'Id'.
	1280,		// PI loop I gain for 'Id'.
	16,			// PI loop P gain for 'Iq'.
	1280,		// PI loop I gain for 'Iq'.
	25,			// throttle regenMax.  Hall effect throttle.  actual low voltage should be about 50 ticks.  0.25v, assuming 5v power supply.  Datasheet says 5% of power supply voltage.
	400,		// throttle regenStart.
	624,		// savedValues.minThrottle start.  (not regen.  What do you call that??
	972,		// savedValues.maxThrottle. Hall effect throttle.  actual high voltage should be about 972 ticks, so clamp it. Max voltage is 4.75v, assuming 5v power supply.  Datasheet says 95% of power supply voltage.
	5,			// throttle fault voltage. Disconnected throttle will pull down due to 100k pull down resistor.  But it will take 0.05 seconds, due to 0.1uF cap on the A/D input.
	0,			// max battery amps.
	0,			// max battery regen amps.
	60,			// precharge time in 0.1 second intervals.
	0			// I compute the CRC below, so this is just garbage here.
};

SavedValuesStruct savedValues = {
16,1280,16,1280,25,400,624,972,5,0,0,60,0
};

extern unsigned int timeOfLastDatastreamTransmission;

// This is always a copy of the data that's in the EE PROM.
// To change EE Prom, modify this, and then copy it to EE Prom.
/*
int EEDataInRamCopy1[] = {1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0};
int EEDataInRamCopy2[] = {11,12,13,24,30,30,30,30,30,40,50,60,780,90,100,60};
int EEDataInRamCopy3[] = {11,12,13,24,30,30,30,30,30,40,50,60,780,90,100,60};
int EEDataInRamCopy4[] = {11,12,13,24,30,30,30,30,30,40,50,60,780,90,100,60};


_prog_addressT EE_addr_Copy1 = 0x7ffc00;
_prog_addressT EE_addr_Copy2 = 0x7ffc20;
_prog_addressT EE_addr_Copy3 = 0x7ffc40;
_prog_addressT EE_addr_Copy4 = 0x7ffc60;

*/
const unsigned int distCorrection[] = {
32768,32758,32748,32738,32728,32718,32709,32699,32689,32680,32670,32660,32651,32641,32632,32622,32613,32603,32594,32585,32575,32566,32557,32548,32539,32530,32521,32512,32503,32494,32485,32476,32467,32458,32449,32441,32432,32423,32415,32406,32398,32389,32381,32372,32364,32355,32347,32339,32330,32322,32314,32306,32298,32290,32282,32274,32266,32258,32250,32242,32234,32226,32218,32211,32203,32195,32188,32180,32173,32165,32158,32150,32143,32135,32128,32121,32113,32106,32099,32092,32085,32077,32070,32063,32056,32049,32042,32036,32029,32022,32015,32008,32002,31995,31988,31982,31975,31968,31962,31955,31949,31942,31936,31930,31923,31917,31911,31905,31898,31892,31886,31880,31874,31868,31862,31856,31850,31844,31838,31832,
31827,31821,31815,31810,31804,31798,31793,31787,31782,31776,31771,31765,31760,31754,31749,31744,31738,31733,31728,31723,31718,31713,31708,31702,31697,31692,31688,31683,31678,31673,31668,31663,31658,31654,31649,31644,31640,31635,31631,31626,31622,31617,31613,31608,31604,31600,31595,31591,31587,31582,31578,31574,31570,31566,31562,31558,31554,31550,31546,31542,31538,31534,31530,31526,31523,31519,31515,31512,31508,31504,31501,31497,31494,31490,31487,31483,31480,31477,31473,31470,31467,31463,31460,31457,31454,31451,31448,31444,31441,31438,
31435,31432,31429,31427,31424,31421,31418,31415,31413,31410,31407,31404,31402,31399,31397,31394,31392,31389,31387,31384,31382,31379,31377,31375,31372,31370,31368,31366,31363,31361,31359,31357,31355,31353,31351,31349,31347,31345,31343,31341,31339,31338,31336,31334,31332,31331,31329,31327,31326,31324,31322,31321,31319,31318,31316,31315,31314,31312,31311,31310,31308,31307,31306,31304,31303,31302,31301,31300,31299,31298,31297,31296,31295,31294,31293,31292,31291,31290,31289,31289,31288,31287,31286,31286,31285,31284,31284,31283,31282,31282,31281,31281,31280,31280,31280,31279,31279,31279,31278,31278,31278,31277,31277,31277,31277,
31277,31277,31277,31276,31276,31276,31276,31276,31277,31277,31277,31277,31277,31277,31277,31278,31278,31278,31278,31279,31279,31280,31280,31280,31281,31281,31282,31282,31283,31283,31284,31285,31285,31286,31287,31287,31288,31289,31290,31290,31291,31292,31293,31294,31295,31296,31297,31298,31299,31300,31301,31302,31303,31304,31305,31306,31308,31309,31310,31311,31313,31314,31315,31317,31318,31319,31321,31322,31324,31325,31327,31328,31330,31331,31333,31335,31336,31338,31340,31341,31343,31345,31347,31348,31350,31352,31354,31356,31358,31360,31362,31364,31366,31368,31370,31372,31374,31376,31378,31380,31382,31384,31387,31389,31391,
31393,31396,31398,31400,31403,31405,31407,31410,31412,31415,31417,31420,31422,31425,31427,31430,31432,31435,31438,31440,31443,31446,31448,31451,31454,31457,31459,31462,31465,31468,31471,31474,31476,31479,31482,31485,31488,31491,31494,31497,31500,31503,31507,31510,31513,31516,31519,31522,31526,31529,31532,31535,31539,31542,31545,31549,31552,31555,31559,31562,31566,31569,31572,31576,31579,31583,31587,31590,31594,31597,31601,31605,31608,31612,31616,31619,31623,31627,31630,31634,31638,31642,31646,31650,31653,31657,31661,31665,31669,31673,31677,31681,31685,31689,31693,31697,31701,31705,31709,31713,31718,31722,31726,31730,31734,
31739,31743,31747,31751,31756,31760,31764,31769,31773,31777,31782,31786,31791,31795,31800,31804,31808,31813,31817,31822,31827,31831,31836,31840,31845,31850,31854,31859,31864,31868,31873,31878,31882,31887,31892,31897,31902,31906,31911,31916,31921,31926,31931,31936,31941,31946,31951,31956,31961,31966,31971,31976,31981,31986,31991,31996,32001,32006,32011,32016,32022,32027,32032,32037,32042,32048,32053,32058,32063,32069,32074,32079,32085,32090,32095,32101,32106,32112,32117,32123,32128,32133,32139,32144,32150,32155,32161,32167,32172,32178,32183,32189,32195,32200,32206,32212,32217,32223,32229,32234,32240,32246,32252,32257,32263,
32269,32275,32281,32286,32292,32298,32304,32310,32316,32322,32328,32334,32339,32345,32351,32357,32363,32369,32375,32382,32388,32394,32400,32406,32412,32418,32424,32430,32436,32443,32449,32455,32461,32467,32474,32480,32486,32492,32499,32505,32511,32518,32524,32530,32537,32543,32549,32556,32562,32569,32575,32581,32588,32594,32601,32607,32614,32620,32627,32633,32640,32646,32653,32660,32666,32673,32679,32686,32693,32699,32706,32713,32719,32726,32733,32739,32746,32753,32759,32766,32773,32780,32787,32793,32800,32807,32814,32821,32827,32834,32841,32848,32855,32862,32869,32876,32883,32890,32896,32903,32910,32917,32924,32931,32938,
32945,32952,32960,32967,32974,32981,32988,32995,33002,33009,33016,33023,33031,33038,33045,33052,33059,33066,33074,33081,33088,33095,33102,33110,33117,33124,33132,33139,33146,33153,33161,33168,33175,33183,33190,33198,33205,33212,33220,33227,33234,33242,33249,33257,33264,33272,33279,33287,33294,33302,33309,33317,33324,33332,33339,33347,33354,33362,33369,33377,33385,33392,33400,33407,33415,33423,33430,33438,33446,33453,33461,33469,33476,33484,33492,33499,33507,33515,33523,33530,33538,33546,33554,33561,33569,33577,33585,33593,33600,33608,33616,33624,33632,33640,33648,33655,33663,33671,33679,33687,33695,33703,33711,33719,33727,
33735,33743,33751,33759,33767,33775,33783,33791,33799,33807,33815,33823,33831,33839,33847,33855,33863,33871,33879,33887,33895,33903,33912,33920,33928,33936,33944,33952,33960,33969,33977,33985,33993,34001,34010,34018,34026,34034,34042,34051,34059,34067,34075,34084,34092,34100,34109,34117,34125,34133,34142,34150,34158,34167,34175,34183,34192,34200,34208,34217,34225,34234,34242,34250,34259,34267,34276,34284,34292,34301,34309,34318,34326,34335,34343,34352,34360,34369,34377,34386,34394,34403,34411,34420,34428,34437,34445,34454,34462,34471,34479,34488,34497,34505,34514,34522,34531,34539,34548,34557,34565,34574,34583,34591,34600,
34608,34617,34626,34634,34643,34652,34660,34669,34678,34687,34695,34704,34713,34721,34730,34739,34748,34756,34765,34774,34782,34791,34800,34809,34818,34826,34835,34844,34853,34861,34870,34879,34888,34897,34906,34914,34923,34932,34941,34950,34959,34967,34976,34985,34994,35003,35012,35021,35030,35038,35047,35056,35065,35074,35083,35092,35101,35110,35119,35128,35137,35146,35154,35163,35172,35181,35190,35199,35208,35217,35226,35235,35244,35253,35262,35271,35280,35289,35298,35307,35316
};

// 512 possible values for sin.  for x in [-1, 1] = [-32767, 32767].  So the scale is about 2^15 - 1.  Let's pretend it's 2^15.  haha.
const int _sin_times32768[] =  
{0, 	402,	804,	1206,	1608,	2009,	2410,	2811,	3212,	3612,	4011,	4410,	4808,	5205,	5602,	5998,	6393,	6786,	7179,	7571,	7962,
8351,	8739,	9126,	9512,	9896,	10278,	10659,	11039,	11417,	11793,	12167,	12539,	12910,	13279,	13645,	14010,	14372,	14732,	15090,	15446,
15800,	16151,	16499,	16846,	17189,	17530,	17869,	18204,	18537,	18868,	19195,	19519,	19841,	20159,	20475,	20787,	21096,	21403,	21705,	22005,
22301,	22594,	22884,	23170,	23452,	23731,	24007,	24279,	24547,	24811,	25072,	25329,	25582,	25832,	26077,	26319,	26556,	26790,	27019,	27245,
27466,	27683,	27896,	28105,	28310,	28510,	28706,	28898,	29085,	29268,	29447,	29621,	29791,	29956,	30117,	30273,	30424,	30571,	30714,	30852,
30985,	31113,	31237,	31356,	31470,	31580,	31685,	31785,	31880,	31971,	32057,	32137,	32213,	32285,	32351,	32412,	32469,	32521,	32567,	32609,
32646,	32678,	32705,	32728,	32745,	32757,	32765,	32767,	32765,	32757,	32745,	32728,	32705,	32678,	32646,	32609,	32567,	32521,	32469,	32412,
32351,	32285,	32213,	32137,	32057,	31971,	31880,	31785,	31685,	31580,	31470,	31356,	31237,	31113,	30985,	30852,	30714,	30571,	30424,	30273,
30117,	29956,	29791,	29621,	29447,	29268,	29085,	28898,	28706,	28510,	28310,	28105,	27896,	27683,	27466,	27245,	27019,	26790,	26556,	26319,
26077,	25832,	25582,	25329,	25072,	24811,	24547,	24279,	24007,	23731,	23452,	23170,	22884,	22594,	22301,	22005,	21705,	21403,	21096,	20787,
20475,	20159,	19841,	19519,	19195,	18868,	18537,	18204,	17869,	17530,	17189,	16846,	16499,	16151,	15800,	15446,	15090,	14732,	14372,	14010,
13645,	13279,	12910,	12539,	12167,	11793,	11417,	11039,	10659,	10278,	9896,	9512,	9126,	8739,	8351,	7962,	7571,	7179,	6786,	6393,
5998,	5602,	5205,	4808,	4410,	4011,	3612,	3212,	2811,	2410,	2009,	1608,	1206,	804,	402,	0,  	-402,	-804,	-1206,	-1608,
-2009,	-2410,	-2811,	-3212,	-3612,	-4011,	-4410,	-4808,	-5205,	-5602,	-5998,	-6393,	-6786,	-7179,	-7571,	-7962,	-8351,	-8739,	-9126,	-9512,
-9896,	-10278,	-10659,	-11039,	-11417,	-11793,	-12167,	-12539,	-12910,	-13279,	-13645,	-14010,	-14372,	-14732,	-15090,	-15446,	-15800,	-16151,	-16499,	-16846,
-17189,	-17530,	-17869,	-18204,	-18537,	-18868,	-19195,	-19519,	-19841,	-20159,	-20475,	-20787,	-21096,	-21403,	-21705,	-22005,	-22301,	-22594,	-22884,	-23170,
-23452,	-23731,	-24007,	-24279,	-24547,	-24811,	-25072,	-25329,	-25582,	-25832,	-26077,	-26319,	-26556,	-26790,	-27019,	-27245,	-27466,	-27683,	-27896,	-28105,
-28310,	-28510,	-28706,	-28898,	-29085,	-29268,	-29447,	-29621,	-29791,	-29956,	-30117,	-30273,	-30424,	-30571,	-30714,	-30852,	-30985,	-31113,	-31237,	-31356,
-31470,	-31580,	-31685,	-31785,	-31880,	-31971,	-32057,	-32137,	-32213,	-32285,	-32351,	-32412,	-32469,	-32521,	-32567,	-32609,	-32646,	-32678,	-32705,	-32728,
-32745,	-32757,	-32765,	-32767,	-32765,	-32757,	-32745,	-32728,	-32705,	-32678,	-32646,	-32609,	-32567,	-32521,	-32469,	-32412,	-32351,	-32285,	-32213,	-32137,
-32057,	-31971,	-31880,	-31785,	-31685,	-31580,	-31470,	-31356,	-31237,	-31113,	-30985,	-30852,	-30714,	-30571,	-30424,	-30273,	-30117,	-29956,	-29791,	-29621,
-29447,	-29268,	-29085,	-28898,	-28706,	-28510,	-28310,	-28105,	-27896,	-27683,	-27466,	-27245,	-27019,	-26790,	-26556,	-26319,	-26077,	-25832,	-25582,	-25329,
-25072,	-24811,	-24547,	-24279,	-24007,	-23731,	-23452,	-23170,	-22884,	-22594,	-22301,	-22005,	-21705,	-21403,	-21096,	-20787,	-20475,	-20159,	-19841,	-19519,
-19195,	-18868,	-18537,	-18204,	-17869,	-17530,	-17189,	-16846,	-16499,	-16151,	-15800,	-15446,	-15090,	-14732,	-14372,	-14010,	-13645,	-13279,	-12910,	-12539,
-12167,	-11793,	-11417,	-11039,	-10659,	-10278,	-9896,	-9512,	-9126,	-8739,	-8351,	-7962,	-7571,	-7179,	-6786,	-6393,	-5998,	-5602,	-5205,	-4808,
-4410,	-4011,	-3612,	-3212,	-2811,	-2410,	-2009,	-1608,	-1206,	-804,	-402, 		0,		0,		0,		0,		0,		0,		0,		0,};
////////////////////////////////////////////////////////////////

volatile realtime_data_type RTData;
volatile unsigned int showDatastreamJustOnce = 0;
volatile unsigned int faultBits = 0;
volatile int vRef1 = 0, vRef2 = 0;
volatile long throttleSum = 0;
volatile int throttle = 0;

volatile int temperatureMultiplier = 8;

volatile int throttleFaultCounter = 0;
volatile unsigned int dataStreamPeriod = 0;
volatile int temperature = 0;
volatile long temperatureSum = 0;
volatile unsigned int counter1ms = 0;
volatile int overCurrentHappened = 0;
volatile int underVoltageHappened = 0;
volatile int ADCurrent1 = 0, ADCurrent2 = 0, ADThrottle = 0, ADTemperature = 0;
//volatile int batteryAmps = 0;
volatile int rotorFluxAngle = 0; 		// This is the rotor flux angle. In [0, 511]
volatile unsigned int rotorFluxAngle_times128 = 0;  // For fine control.
volatile int rotorTimeConstantIndex = 64;  // 70 corresponds to rotorTimeConstant of 0.110.
volatile int RPS_times64 = 0; // range [-3200, 3200], where 3200 corresponds to 200rev/sec = 12,000RPM, and 0 means 0RPM.
volatile int slipSpeedRPS_times64 = 0;
volatile int rotorFluxRPS_times64 = 0;
volatile int angleChange_times128 = 0;
volatile piType pi_Iq, pi_Id;

volatile unsigned int startTimeInterrupt = 0, elapsedTimeInterrupt = 0;		// 
volatile long i_alpha = 0;
volatile long i_beta = 0;
volatile int Id = 0;
volatile int Iq = 0;
volatile int Ia = 0, Ib = 0, Ic = 0;
volatile int magnetizingCurrent = 0;
volatile int Vd = 0, Vq = 0;
volatile int tempVd = 0;
volatile int tempVq = 0;
volatile long Vd_times16 = 0;
volatile long Vq_times16 = 0;

volatile int numberOfPolePairs = 2;	// 2 pole pairs in my case.
volatile int Va = 0, Vb = 0, Vc = 0;
volatile int IdRef = 500;	// in the range [0, 4096]
volatile int IqRef = 500; // in the range [-4096, 4096]

int IdArray[280]; // id and iq are in 
int IqArray[280];

/*
extern int TransmitString(char* str);
extern void u16x_to_str(char *str, unsigned val, unsigned char digits);
extern void u16_to_str(char *str, unsigned val, unsigned char digits);
extern void ShowMenu(void);
extern void ProcessCommand(void);
*/
void FetchRTData();
void InitTimers();
void InitIORegisters(void);
void Delay(unsigned int);
void DelayTenthsSecond(unsigned int time);
void DelaySeconds(unsigned int time);
void ReadADInputs();
void GrabADResults();
void InitADAndPWM();
void InitDiscreteADConversions();
void GetVRefs();
void DelaySeconds(unsigned int seconds);
void DecimalToString(int number, int length);
char IntToCharHex(unsigned int i);
void InitCNModule();
void InitPIStruct();
void ClearAllFaults();  // clear the flip flop fault and the desat fault.
void ClearDesatFault();
void ClearFlipFlop();
void ComputeRotorFluxAngle();
void SpaceVectorModulation();
void ClampVdVq();
void Delay1uS();
void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void);
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void);

void MoveDataFromEEPromToRAM();
void EESaveValues();

//char RTDataString[] = "TH=xxx Id=xxx CR=xxx Iq=xxx D1=xxxx D2=xxxx D3=xxxx HS=xxx RT=xxx FB=xx BA=xxx\r\n";

int main() {
	InitIORegisters();
//	long localThrottleLong = 0;

//	MoveDataFromEEPromToRAM();
//	EESaveValues();

	
	InitTimers();  // Now timer1 is running at 117KHz
	Delay(DELAY_200MS_SLOW_TIMER);  // 200mS, just to let voltages settle.	

	InitCNModule();
	InitDiscreteADConversions();
	GetVRefs();
	
//	InitUART2();  // Now the UART is running.
	InitPIStruct();

	// High pedal lockout. Wait until they aren't touching the throttle before starting the precharge sequence.
/*	do {
		ReadADInputs();
	} while (ADThrottle < savedValues.minRegen || ADThrottle > savedValues.minThrottle);  // I'm using a wig wag hall effect throttle for testing.
*/
	O_LAT_PRECHARGE_RELAY = 1;  // 1 means close the relay.  Now the cap is filling up.
	DelayTenthsSecond(savedValues.prechargeTime);
	// High pedal lockout. Wait until they aren't touching the throttle before closing the main contactor.
/*
	do {
		ReadADInputs();
	} while (ADThrottle < savedValues.minRegen || ADThrottle > savedValues.minThrottle);  // I'm using a wig wag hall effect throttle.
*/
	O_LAT_CONTACTOR = 1;	// close main contactor.
	DelayTenthsSecond(2);   // delay 0.2 seconds, to give the contactor a chance to close.  Then, there will be no current going through the precharge relay.
	O_LAT_PRECHARGE_RELAY = 0;  // open precharge relay once main contactor is closed.

	InitADAndPWM();		// Now the A/D is triggered by the pwm period, and the PWM interrupt is enabled.
	ClearAllFaults();
/*	ClearReceiveBuffer();
	ShowMenu(); 	// serial show menu.
*/
	while(1) {
//		ProcessCommand();  // If there's a command to be processed, this does it.  haha.
		if (TMR2 >= 117) {  // TMR3:TMR2 is a 32 bit timer, running at 117KHz.  So, 117 ticks of TMR2 (the low 16 bits) is about 1ms.
			TMR2 = 0;
			counter1ms++;
		}
		// add non time-critical code below 
		// fetch real time data
/*		FetchRTData();
		// if datastreamPeriod not zero display rt data at specified interval

		if (dataStreamPeriod) {
			if ((counter1ms - timeOfLastDatastreamTransmission) >= dataStreamPeriod) {
				if (showDatastreamJustOnce) {
					dataStreamPeriod = 0;  // You showed it once, now stop the stream.
				}
				// 0         1         2         3         4         5         6         7         8         9
				// 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
				// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xx   // That's the excel version.
				timeOfLastDatastreamTransmission += dataStreamPeriod;
				u16_to_str(&RTDataString[0], RTData.throttleRaw, 4);
				u16_to_str(&RTDataString[10], RTData.ADCurrent1, 4);
				u16_to_str(&RTDataString[15], RTData.ADCurrent2, 4);				
				u16_to_str(&RTDataString[20], RTData.raw_hs_temp, 4);  
				u16_to_str(&RTDataString[25], RTData.IqRef, 4);  // Average throttle.  In [0, 8192]?  I want to match range for Iq...  In excel, change to [-4096, 4096]
				u16_to_str(&RTDataString[25], RTData.Id, 4);  // in [0, 8192], which is actually [-4096, 4096], where negative means regen.
				u16_to_str(&RTDataString[30], RTData.Iq, 4);
				u16_to_str(&RTDataString[35], RTData.pdc1, 4);  // in [0,3000]
				u16_to_str(&RTDataString[40], RTData.pdc2, 4);
				u16_to_str(&RTDataString[45], RTData.pdc3, 4);
				u16_to_str(&RTDataString[55], RTData.RPM, 4);
				u16_to_str(&RTDataString[50], RTData.batteryAmps, 4);
				u16x_to_str(&RTDataString[60], faultBits, 2);
				TransmitString(RTDataString);
			}
		}
*/
		// let the interrupt take care of the rest...
		#ifndef DEBUG
		ClrWdt();
		#endif
	}
}

//---------------------------------------------------------------------
// The ADC sample and conversion is triggered by the PWM period.
//---------------------------------------------------------------------
// This runs at 10kHz.
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void) {
	static long vBetaSqrt3_times32768 = 0;
	static long vAlpha_times32768 = 0;
	static int vBeta = 0;
	static int revCounter = 0;	// revCounter increments at 10kHz.  When it gets to 78, the number of ticks in POSCNT is extremely close to the revolutions per seoond * 16.
								// So, the motor mechanical speed will be computed every 1/128 seconds, and will have a range of [0, 3200], where 3200 corresponds to 12000rpm.	

	static int throttleCounter = 0;
	static long cos_theta_times32768 = 0;
	static long sin_theta_times32768 = 0;
	static int rotorFluxAnglePlus90 = 0;
	static int IdIqArrayCounter = 0;
	static unsigned int counter10k = 0;

	startTimeInterrupt = TMR4;
    IFS0bits.ADIF = 0;  	// Interrupt Flag Status Register. Pg. 142 in F.R.M.
							// ADIF = A/D Conversion Complete Interrupt Flag Status bit.  
							// ADIF = 0 means we are resetting it so that an interrupt request has not occurred.
	counter10k++;
	// CH0 corresponds to ADCBUF0. etc...
	// CH0=AN7, CH1=AN0, CH2=AN1, CH3=AN2. 
	// AN0 = CH1 = ADThrottle
	// AN1 = CH2 = ADCurrent1
	// AN2 = CH3 = ADCurrent2
	// AN7 = CH0 = ADTemperature

	ADCurrent1 = ADCBUF2;
	ADCurrent2 = ADCBUF3;
	Ia = ADCurrent1;	// CH2 = ADCurrent1
	Ib = ADCurrent2;		// CH3 = ADCurrent2

	// so dense... so glorious.  Covers positive and negative RPM.
	revCounter++;
	if (revCounter >= 312) {  // Compute rpm at 32 times a second.
		RPS_times64 = POSCNT;	// if POSCNT is 0x0FFFF due to THE MOTOR IS GOING BACKWARDS, RPS_times16 would be -1, since it's of type signed short.  So, it's all good.  Negative RPM is covered.
		POSCNT = 0;
		revCounter = 0;
	}

	throttleSum += ADCBUF1;
	temperatureSum += ADCBUF0;
	throttleCounter++;
	if (throttleCounter >= 64) {
		throttleCounter = 0;
		throttle = (throttleSum >> 6);  // in [0, 1023].
		temperature = (temperatureSum >> 6); // in [0,1023].
		throttleSum = 0;
		temperatureSum = 0;

		if (throttle <= savedValues.throttleFault) {
			throttleFaultCounter++;
			if (throttleFaultCounter >= THROTTLE_FAULT_COUNTS) {
				faultBits |= THROTTLE_FAULT;
			}
		}
		else {
			if (throttleFaultCounter > 0) {  // Hurray, no fault, so decrement the fault counter if necessary.
				throttleFaultCounter--;
			}
		}

		// if throttle < savedValues.maxThrottleRegen.  I think I'll work my way left to right.  ThrottleFaultVoltage < ThrottleMaxRegen < ThrottleMinRegen < ThrottleMin < ThrottleMax 
		if (throttle < savedValues.maxRegen) {  //First clamp it below.
			throttle = savedValues.maxRegen;	
		}
		else if (throttle > savedValues.maxThrottle) {	// And clamp it above!  
			throttle = savedValues.maxThrottle;
		}
		if (throttle < savedValues.minRegen) {  // It's in regen territory.  Map it from [savedValues.maxThrottleRegen, savedValues.minThrottleRegen) to [-4096, 0)
			throttle -= savedValues.minRegen;  // now it's in the range [maxThrottleRegen - minThrottleRegen, 0]
			//throttle = -(((long)throttle) << 12) / ((long)(savedValues.maxRegen - savedValues.minRegen));
			throttle =	-__builtin_divsd(((long)throttle) << 12, savedValues.maxRegen - savedValues.minRegen);
		}
		else if (throttle <= savedValues.minThrottle) { // in the dead zone!
			throttle = 0;
		}
		else { // <= throttle max is the only other option!  Map the throttle from (savedValues.minThrottle, savedValues.maxThrottle] to (0,4096]
			throttle -= savedValues.minThrottle;
			throttle = __builtin_divsd(((long)throttle) << 12, savedValues.maxThrottle - savedValues.minThrottle);
		}
		if (temperature > THERMAL_CUTBACK_START) {  // Force the throttle to cut back.
			temperatureMultiplier = (THERMAL_CUTBACK_END - THERMAL_CUTBACK_START) >> 3;  // 0 THROUGH 7.
			if (temperatureMultiplier >= 7)
				temperatureMultiplier = 0;
			else {
				// temperatureMultiplier is now 6 to 0 (for 1/8 to 7/8 current)
				temperatureMultiplier = 7 - temperatureMultiplier;
				// temperatureMultiplier is now 1 for 1/8, 2 for 2/8, ..., 7 for 7/8, etc.
			}
		}
		else {
			temperatureMultiplier = 8;	// Allow full throttle.
		}
		IqRef = __builtin_mulss(throttle,temperatureMultiplier) >> 3;
//		IdRef = IqRef;
//		if (IdRef < 0) IdRef = -IdRef;
		IdRef = 400;
	}



	Ia -= vRef1;
	Ib -= vRef2;

	Ia = -Ia;
	Ib = -Ib;
//	if (i_alpha < -255) i_alpha = -255;
//	else if (i_alpha >255) i_alpha = 255;
//	if (Ib < -255) Ib = -255;
//	else if (Ib > 255) Ib = 255;
	Ia <<= 4;		// now it's in the range [-4096,4096] if it's in the range [-100Amp, 100Amp] = [-256,256] for the LEM HASS 50-s.
	Ib <<= 4;

	// Now, do clarke transform.  First, take the 3 vectors, 120 degrees apart, and add them to
	// get a new vector, and project that vector onto the x and y axis.  The x-axis component is called i_alpha.  y-axis component is called i_beta.

	// clarke transform, scaled down by 2/3 to simplify it:
	// i_alpha = i_a
	// 1/sqrt(3) * 2^16 = 37837
	i_alpha = Ia;
	i_beta = (((long)(2*Ib + i_alpha)) * 37837L) >> 16;  // 1/sqrt(3) * (i_a + 2 * Ib).  However, I already multiplied by 2 above...
	// End of clarke transform.
	// this uses Id and Iq, found below.  So, initialize them to something.  I'll have Id start as 0, and Iq start as 0.
	ComputeRotorFluxAngle();  // Now, the global variable "rotorFluxAngle" is loaded.  In [0,511].

	// park transform.
	// rotorFluxAngle is in [0, 511].
	// sin(theta + 90 degrees) = cos(theta).
	// I want the 2 angles to be in [0, 511] so I can use the lookup table.
	rotorFluxAnglePlus90 = ((rotorFluxAngle + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
															// and then do "& 511" to make it wrap around if overflow occurred.
	cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
	sin_theta_times32768 = _sin_times32768[rotorFluxAngle];  // 
	Id = (i_alpha*(long)cos_theta_times32768 + i_beta*(long)sin_theta_times32768) >> 15; 	
	Iq = (-i_alpha*(long)sin_theta_times32768 + i_beta*(long)cos_theta_times32768) >> 15; 
	
//	if (Id < IdRef) Vd_times16++;
//	else if (Id > IdRef) Vd_times16--;
//	if (Iq < IqRef) Vq_times16++;
//	else if (Iq > IqRef) Vq_times16--;
	// 
	if (IdIqArrayCounter >= 280) {
	 	IdIqArrayCounter = 0;
		if (counter10k > 20000) {  // 5 seconds.
			counter10k = 0;
			Nop();
			Nop();
			Nop();
			Nop();
			Nop();
			Nop();
			rotorTimeConstantIndex = 76;
//			rotorTimeConstantIndex++;
			if (rotorTimeConstantIndex > 99) {
				rotorTimeConstantIndex = 0;
			}
		}
	}
	IdArray[IdIqArrayCounter] = Id;
	IqArray[IdIqArrayCounter] = Iq;
	IdIqArrayCounter++;
	
		// error = reference - feedback.
	if (Iq < IqRef) Vq+=4;
	else if (Iq > IqRef) Vq-=4;
	if (Id < IdRef) Vd+=4;
	else if (Id > IdRef) Vd-=4;
	ClampVdVq();

/*
	pi_Id.error_new = IdRef - Id;
	pi_Iq.error_new = IqRef - Iq;
	// execute PI loop
	// first, K1 = Kp << 10;   But this was done at the start of the program.
	// second, K2 = Ki - K1;
	if (IdRef == 0) {
		pi_Id.pwm = 0;
	}
	else {
		pi_Id.pwm += (pi_Id.K1 * pi_Id.error_new) + (pi_Id.K2 * pi_Id.error_old);
	}
	if (IqRef == 0) {
		pi_Iq.pwm = 0;
	}
	else {
		pi_Iq.pwm += (pi_Iq.K1 * pi_Iq.error_new) + (pi_Iq.K2 * pi_Iq.error_old);
	}
	pi_Id.error_old = pi_Id.error_new; 
	pi_Iq.error_old = pi_Iq.error_new;

	Vd = pi_Id.pwm >> 16;	
	Vq = pi_Iq.pwm >> 16;

	tempVd = Vd;
	tempVq = Vq;
	ClampVdVq();
	if (Vd != tempVd) {
		pi_Id.pwm = ((long)Vd) << 16;
	}
	if (Vq != tempVq) {
		pi_Iq.pwm = ((long)Vq) << 16;
	}
*/
	//vAlpha_times32768 = (Vd*cos_theta_times32768 - Vq*sin_theta_times32768);	// shift right 15 because sin and cos have been shifted left by 15.
	//vBeta =  (Vd*sin_theta_times32768 + Vq*cos_theta_times32768) >> 15;	//
	vAlpha_times32768 = __builtin_mulss(Vd, cos_theta_times32768) - __builtin_mulss(Vq, sin_theta_times32768);
	vBeta = (__builtin_mulss(Vd, sin_theta_times32768) + __builtin_mulss(Vq, cos_theta_times32768)) >> 15;
	
	// Now do the inverse Clarke transform, with a scaling factor of 3/2.  On the clarke transform, there was a scaling factor of 2/3 to simplify things.
	//  Va = v_alpha
    //	Vb = 1/2*(-v_alpha + sqrt(3)*vBeta)
    //  Vc = 1/2*(-v_alpha - sqrt(3)*vBeta);
	Va = vAlpha_times32768 >> 15;
	//vBetaSqrt3_times32768 = (56756*vBeta);  // 56756 = sqrt(3)*(2^15).
	vBetaSqrt3_times32768 = __builtin_mulus(56756u, vBeta);  // 56756 = sqrt(3)*(2^15).
	
	Vb = (-vAlpha_times32768 + vBetaSqrt3_times32768) >> 16;  // scaled up by 2^15, so shift down by 15.  But you also must divide by 2 at the end as part of the inverse clarke.
	Vc = (-vAlpha_times32768 - vBetaSqrt3_times32768) >> 16;
//	if (IqRef != 0) {
		SpaceVectorModulation();
//	}
//	else {
//		PDC1 = 0;
//		PDC2 = 0;
//		PDC3 = 0;
//	}
//	PDC1 = 200;
//	PDC2 = 400;
//	PDC3 = 800;
	elapsedTimeInterrupt = TMR4 - startTimeInterrupt;

}

void ComputeRotorFluxAngle() {
	static int temp = 0;
	// = loopPeriod / rotorTimeConstant * 2^18.  loopPeriod is 0.0001 seconds, because it's being run at 10KHz. Rotor time constants range from 0.040 to 0.140 seconds.
	// After using an element from this array, you must divide the result by 2^18!!!
	// rotorTimeConstantArray1[0] corresponds to a rotorTimeConstant of 0.040 seconds.  rotorTimeConstantArray2[100] <=> rotor time constant of 0.140.
	static int rotorTimeConstantArray1[] = {
		655,639,624,610,596,583,570,558,546,535,524,514,504,495,485,477,468,460,452,444,437,430,423,416,410,403,397,391,386,380,374,369,364,359,354,350,345,340,336,332,328,324,320,316,312,308,305,301,298,295,291,288,285,282,279,276,273,270,267,265,262,260,257,255,252,250,247,245,243,240,238,236,234,232,230,228,226,224,222,220,218,217,215,213,211,210,208,206,205,203,202,200,199,197,196,194,193,191,190,189,187
	};
	// (1/rotorTimeConstant) * 1/(2*pi) * 2^13.  I'm trying to keep all of them in integer range.
	// rotorTimeConstantArray2[0] corresponds to a rotorTimeConstant of 0.040 seconds.  rotorTimeConstantArray2[100] = 0.140.
	static int rotorTimeConstantArray2[] = {
		32595,31800,31043,30321,29632,28973,28343,27740,27162,26608,26076,25565,25073,24600,24144,23705,23282,22874,22479,22098,21730,21374,21029,20695,20372,20058,19755,19460,19173,18896,18626,18363,18108,17860,17619,17384,17155,16932,16715,16504,16297,16096,15900,15708,15521,15339,15160,14986,14816,14649,14487,14327,14172,14019,13870,13724,13581,13441,13304,13170,13038,12909,12782,12658,12537,12417,12300,12185,12072,11961,11853,11746,11641,11538,11437,11337,11240,11144,11049,10956,10865,10775,10687,10600,10514,10430,10348,10266,10186,10107,10029,9953,9877,9803,9730,9658,9587,9517,9448,9380,9313
	};
//;	 Physical form of equations:
//;  Magnetizing current (amps):
//;     Imag = Imag + (fLoopPeriod/fRotorTmConst)*(Id - Imag)
//;
//;  Slip speed in RPS:
//;     VelSlipRPS = (1/fRotorTmConst) * (Iq/Imag) / (2*pi)
//;
//;  Rotor flux speed in RPS:
//;     VelFluxRPS = iPoles * VelMechRPS + VelSlipRPS
//;
//;  Rotor flux angle (radians):
//;     AngFlux = AngFlux + fLoopPeriod * 2 * pi * VelFluxRPS

// ****For divide, use this:  	int quot =	 __builtin_divsd(long numerator, int denominator);****
// ****For multiply, use this:  long prod =   __builtin_mulus(unsigned left,  int right);****  or muluu, or mulsu, or mulss.

//; 1.  Magnetizing current (amps):
//;     Imag = Imag + (fLoopPeriod/fRotorTmConst)*(Id - Imag)
//      rotorTimeConstantArray[]'s entries have been scaled up by 2^18 to give more resolution for the rotor time constant.  I scaled by 18, because that allowed incrementing rotor time constant by 0.01 seconds.
//	magnetizingCurrent += ((rotorTimeConstantArray1[rotorTimeConstantIndex] * (Id - magnetizingCurrent))) >> 18; 
	magnetizingCurrent += (__builtin_mulus(rotorTimeConstantArray1[rotorTimeConstantIndex], (Id - magnetizingCurrent)) >> 18); 

// 2.  Compute the slip speed in revolutions per second.
//;    Slip speed in RPS:
//;    VelSlipRPS = (1/fRotorTmConst) * (Iq/Imag) / (2*pi)
//     rotorTimeConstantArray2[] entries are 1/fRotorTmConst * 1/(2*pi) * 2^13.  I couldn't scale any higher than 2^13 so as to keep them integers.
//	   VelSlipRPS = (ARRAY[] * Iq) / Imag.
//	   Let temp = (ARRAY[] * Iq).  Then, do the bit shift down first, and the divide by magnetizing current afterwards to prevent the loss of resolution.
	temp = __builtin_mulus(rotorTimeConstantArray2[rotorTimeConstantIndex], Iq) >> 7; // Must scale down by 2^13 if you want units to be rev/sec.  But that's too grainy.  So, let's only scale down by 2^7 so you get slip speed in rev/sec * 64, rather than just rev/sec
																						 // Shift down by 7 before the division below, so the division will have int as the answer.
																						 // That way, you can use the __builtin_div instead of /.

	// we need temp to be an integer, to guarantee that temp / magnetizingCurrent results in an integer, so we can do a fast divide here.
	if (magnetizingCurrent == 0) magnetizingCurrent = 1;  // Can't divide by zero!!  See if you can figure out a better solution later.
//	slipSpeedRPS_times64 = temp / magnetizingCurrent; 
	slipSpeedRPS_times64 = __builtin_divsd(temp, magnetizingCurrent);
	// clamp slipSpeed later... 
	
	if (slipSpeedRPS_times64 > 7000) slipSpeedRPS_times64 = 7000;
	if (slipSpeedRPS_times64 < -7000) slipSpeedRPS_times64 = -7000;
//; 3. Rotor flux speed in RPS:
//;    VelFluxRPS = iPoles * VelMechRPS + VelSlipRPS
//     RPS_times64 was gloriously found in the main interrupt. It is in [-12800, 12800], which means [-12000RPM, 12000RPM].  I will make sure that normal driving is positive rpm.
//     numberOfPolePairs is 2 in the case of my motor, because the RPM is listed as 1700 with 60 Hz 3 phase input.  1 pole pair would list the rpm as around 3400 with 60 Hz 3 phase input.
//	rotorFluxRPS_times64 = numberOfPolePairs * RPS_times64 + slipSpeedRPS_times64;  // There's no danger of integer overflow, so just do normal multiply.
		rotorFluxRPS_times64 = 2 * RPS_times64 + slipSpeedRPS_times64;  // There's no danger of integer overflow, so just do normal multiply.

//;  Rotor flux angle (radians):
//;     AngFlux = AngFlux + fLoopPeriod * 2 * pi * VelFluxRPS
//;  Rotor flux angle (0 to 511 ticks):
//      AngFlux = AngFlux + fLoopPeriod * 512 * VelFluxRPS.  
//   Now, I don't have VelFluxRPS.  Too darn grainy.  I found rotorFluxRPS_times64 above.  So.....
//      AngFlux = AngFlux + fLoopPeriod * 8 * rotorFluxRPS_times64, because 8 * 64 = 512.
//   OK, fLoopPeriod = 0.0001 sec.  I don't want to divide here, so let's do a trick.
//   0.0001sec * 8 * 2^26 = 53687.09.  So, I'll just multiply by 53687, and shift down by 2^26 afterwards.  Actually, let's keep 2^7 worth of extra resolution, because angleChange was too grainy before.
//	angleChange_times128 = (53687u * rotorFluxRPS_times16) >> 19;  // must shift down by 26 eventually, but let's keep a higher resolution here.  So, only shift down by 19.  Keeping 7 bits.
	angleChange_times128 = __builtin_mulus(53687u, rotorFluxRPS_times64) >> 19;  // must shift down by 24 eventually, but let's keep a higher resolution here.  So, only shift down by 17.  Keeping 7 bits.

	if (angleChange_times128 >= 25000 || angleChange_times128 <= -25000) {  // This may be unnecessary.  I might not need to check negative angle changes?
		faultBits |= ROTOR_FLUX_ANGLE_FAULT;  // This should NEVER happen.  It implies a rotor flux RPM of like 120,000!
	}
	rotorFluxAngle_times128 += angleChange_times128;  // if it overflows, so what.  it will wrap back around.  To go from [0,65536] --> [0,512], divide by by 128.  higher resolution rotor flux angle saved here.
	rotorFluxAngle = rotorFluxAngle_times128 >> 7;
}

void ClampVdVq() {
	unsigned int VdPos, VqPos;
	unsigned int index;
	unsigned int fastDist;
	unsigned int r, scale;

	if (Vd < 0) {
		VdPos = -Vd;
		if (Vq < 0) {  // Vd < 0, Vq < 0
			VqPos = -Vq;
			if (VqPos < VdPos) { // Vd < 0, Vq < 0, |Vq| < |Vd|
//				index = (((long)VqPos) << 10) / VdPos;	// Multiply by 1024 because the array has 1024 elements in it.
				index = __builtin_divud(((long)VqPos) << 10, VdPos);
				fastDist = VqPos + VdPos - (VqPos >> 1) - (VqPos >> 2) + (VqPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = -((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(-VqPos, scale) >> 16;
//					Vd = -((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(-VdPos, scale) >> 16;
				}
				else return;
			}
			else {  // Vd < 0, Vq < 0, |Vq| >= |Vd|
//				index = (((long)VdPos) << 10) / VqPos; 
				index = __builtin_divud(((long)VdPos) << 10, VqPos); 
				fastDist = VdPos + VqPos - (VdPos >> 1) - (VdPos >> 2) + (VdPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = -((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(-VqPos, scale) >> 16;
//					Vd = -((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(-VdPos, scale) >> 16;
				}
				else return;
			}
		}
		else { // Vd < 0, Vq >= 0
			VqPos = Vq;
			if (VqPos < VdPos) { // Vd < 0, Vq >= 0, |Vq| < |Vd|
//				index = (((long)VqPos) << 10) / VdPos; 
				index = __builtin_divud(((long)VqPos) << 10, VdPos); 
				fastDist = VqPos + VdPos - (VqPos >> 1) - (VqPos >> 2) + (VqPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = ((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(VqPos, scale) >> 16;
//					Vd = -((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(-VdPos, scale) >> 16;
				}
				else return;
			}
			else { // Vd < 0, Vq >= 0, |Vq| >= |Vd|
//				index = (((long)VdPos) << 10) / VqPos; 
				index = __builtin_divud(((long)VdPos) << 10, VqPos); 
				fastDist = VdPos + VqPos - (VdPos >> 1) - (VdPos >> 2) + (VdPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = ((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(VqPos, scale) >> 16;
//					Vd = -((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(-VdPos, scale) >> 16;
				}
				else return;
			}
		}
	}
	else {  // Vd >= 0
		VdPos = Vd;
		if (Vq < 0) { // Vd >= 0, Vq < 0
			VqPos = -Vq;
			if (VqPos < VdPos) { // Vd >= 0, Vq < 0, |Vq| < |Vd|
//				index = (((long)VqPos) << 10) / VdPos;
				index = __builtin_divud(((long)VqPos) << 10, VdPos); 
				fastDist = VqPos + VdPos - (VqPos >> 1) - (VqPos >> 2) + (VqPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = -((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(-VqPos, scale) >> 16;
//					Vd = ((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(VdPos, scale) >> 16;
				}
				else return;
			}
			else { // Vd >= 0, Vq < 0, |Vq| >= |Vd|
//				index = (((long)VdPos) << 10) / VqPos; 
				index = __builtin_divud(((long)VdPos) << 10, VqPos); 
				fastDist = VdPos + VqPos - (VdPos >> 1) - (VdPos >> 2) + (VdPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = -((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(-VqPos, scale) >> 16;
//					Vd = ((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(VdPos, scale) >> 16;
				}
				else return;
			}
		}
		else { // Vd >= 0, Vq >= 0
			VqPos = Vq;
			if (VqPos < VdPos) { // Vd >= 0, Vq >= 0, |Vq| < |Vd|
//				index = (((long)VqPos) << 10) / VdPos; 
				index = __builtin_divud(((long)VqPos) << 10, VdPos); 
				fastDist = VqPos + VdPos - (VqPos >> 1) - (VqPos >> 2) + (VqPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = ((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(VqPos, scale) >> 16;
//					Vd = ((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(VdPos, scale) >> 16;
				}
				else return;
			}
			else { // Vd >= 0, Vq >= 0, |Vq| >= |Vd|
				if (Vd == 0) {if (Vq == 0) return;}  // I don't want to divide by zero below.
//				index = (((long)VdPos) << 10) / VqPos; 
				index = __builtin_divud(((long)VdPos) << 10, VqPos); 
				fastDist = VdPos + VqPos - (VdPos >> 1) - (VdPos >> 2) + (VdPos >> 4);
//				r = (distCorrection[index]*fastDist) >> 15;
				r = __builtin_muluu(distCorrection[index], fastDist) >> 15;
				if (r > R_MAX) {
//					scale = R_MAX_TIMES_65536 / r;
					scale = __builtin_divud(R_MAX_TIMES_65536, r);
//					Vq = ((VqPos*scale) >> 16);
					Vq = __builtin_mulsu(VqPos, scale) >> 16;
//					Vd = ((VdPos*scale) >> 16);
					Vd = __builtin_mulsu(VdPos, scale) >> 16;
				}
				else return;
			}
		}
	}
}

void SpaceVectorModulation() { 
	static int ab, ac, bc, ca, ba, cb, abPos, acPos, bcPos; 
	ab = Va - Vb;
	ac = Va - Vc;
	bc = Vb - Vc;
	
	if (ab < 0) {
		abPos = -ab;
	}
	else {
		abPos = ab;
	}
	if (ac < 0) {
		acPos = -ac;
	}
	else {
		acPos = ac;
	}
	if (bc < 0) {
		bcPos = -bc;
	}
	else {
		bcPos = bc;
	}
	if (abPos >= acPos && abPos >= bcPos) {  // abPos is biggest.  
		if (ac < 0) { // you are in Q3
			// ac = Va. bc = Vb as far as labeling goes on "SpaceVectorModulation.pcb"
			PDC1 = bc - ac;
			PDC2 = 0;
			PDC3 = bc;
		}
		else {  // You are in Q6.
			PDC1 = 0;
			PDC2 = ac - bc;
			PDC3 = ac;
		}
	}
	else if (acPos >= abPos && acPos >= bcPos) { // 
		// acPos is biggest.
		cb = -bc;

		if (ab < 0) {  // Q4. ab = Va. cb = Vc
			PDC1 = cb - ab;
			PDC2 = cb;
			PDC3 = 0;
		}
		else {  // Q1. ab = Va. cb = Vc.
			PDC1 = 0;
			PDC2 = ab;
			PDC3 = ab - cb;
		}
	}
	else { // if (bcPos >= acPos && bcPos >= abPos) {  // bcPos is biggest.
		ca = -ac;
		ba = -ab;
		if (ba < 0) { // Q5. ba = Vb. ca = Vc. 
			PDC1 = ca;
			PDC2 = ca - ba;
			PDC3 = 0;
		}
		else {	// Q2. ba = Vb. ca = Vc.
			PDC1 = ba;
			PDC2 = 0;
			PDC3 = ba - ca;
		}
	}
}


void ClearAllFaults() {
	ClearDesatFault();
	ClearFlipFlop();
}

void ClearFlipFlop() {
	O_LAT_CLEAR_FLIP_FLOP = 0;  // Really like 100nS would be enough.
	Delay1uS();
	O_LAT_CLEAR_FLIP_FLOP = 1;	
}

void ClearDesatFault() {	// reset must be pulled low for at least 1.2uS.
	int i = 0;
	O_LAT_CLEAR_DESAT = 0; 	// FOD8316 Datasheet says low for at least 1.2uS.  But then the stupid fault signal may not be cleared for 20 whole uS!
	Delay1uS(); Delay1uS(); Delay1uS();
	O_LAT_CLEAR_DESAT = 1;

	for (i = 0; i < 30; i++) {  // Now, let's waste more than 30uS waiting around for the desat fault signal to be cleared.
		Delay1uS();
	} // 30uS better be long enough!
}
void Delay1uS() {  // Assuming 30MIPs.
	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop();
	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop();
}

void InitPIStruct() {
	pi_Id.K1 = (((long)savedValues.Kp_Id) << 10);
	pi_Id.K2 = ((long)savedValues.Ki_Id) - pi_Id.K1;
	pi_Id.error_old = 0;
	pi_Id.error_new = 0;
	pi_Id.pwm = 0;  // ???

	pi_Iq.K1 = (((long)savedValues.Kp_Iq) << 10);
	pi_Iq.K2 = ((long)savedValues.Ki_Iq) - pi_Id.K1;
	pi_Iq.error_old = 0;
	pi_Iq.error_new = 0;
	pi_Iq.pwm = 0;  // ???
}

// Make sure this is only called after you know the previous conversion is done.  Right now, the conversion is taking ?? uS, so call this
// when ?? us has gone by after starting "StartADConversion()".
// Run this just before starting the next ADConversion.  It will be a lag, but I'll know what's what.  haha.
void GrabADResults() {
	 ADTemperature = ADCBUF0;
	 ADThrottle = ADCBUF1;
	 ADCurrent1 = ADCBUF2;
	 ADCurrent2 = ADCBUF3;
	// AN0 = CH1 = ADThrottle
	// AN1 = CH2 = ADCurrent1
	// AN2 = CH3 = ADCurrent2
	// AN6 = CH0 = ADTemperature
}
// This is the slow one, when you just want to start a conversion, and wait for the results.
void ReadADInputs() {
	ADCON1bits.SAMP = 1; // start sampling ...
	Delay(256); // for 4mS
	ADCON1bits.SAMP = 0; // Stop sampling, and start converting.
	while (!ADCON1bits.DONE) {
		ClrWdt(); // conversion done?
	}
	GrabADResults();
}

void GetVRefs() {
	int i, sum1 = 0, sum2 = 0; //, sum3 = 0;

	for (i = 0; i < 32; i++) {
		ReadADInputs();
		sum1 += ADCurrent1;
		sum2 += ADCurrent2;
	}
	vRef1 = (sum1 >> 5);
	vRef2 = (sum2 >> 5);

	if (vRef1 < 512 - 50 || vRef1 > 512 + 50 || 
		vRef2 < 512 - 50 || vRef2 > 512 + 50) {
		faultBits |= VREF_FAULT;
	}
}

void InitTimers() {
	T1CON = 0;  // Make sure it starts out as 0.
	T1CONbits.TCKPS = 0b11;  // prescale of 256.  So, timer1 will run at 117.1875 KHz if Fcy is 30.000MHz.
	PR1 = 0xFFFF;  // 
	T1CONbits.TON = 1; // Start the timer.

	T2CONbits.T32 = 1;  // 32 bit mode.
	T2CONbits.TCKPS = 0b11;  // 1:1 prescaler.
	T2CONbits.TCS = 0;  	// use internal 30MHz Fcy for the clock.
	PR3 = 0x0FFFF;		// HIGH 16 BITS.
	PR2 = 0x0FFFF;		// low 16 bits.
	// Now, TMR3:TMR2 makes up the 32 bit timer, running at 117KHz.

	T4CONbits.T32 = 1;  // 32 bit mode.
	T4CONbits.TCKPS = 0b00;  // 1:1 prescaler.
	T4CONbits.TCS = 0;  	// use internal 30MHz Fcy for the clock.
	PR5 = 0x0FFFF;		// HIGH 16 BITS.
	PR4 = 0x0FFFF;		// low 16 bits.

	T2CONbits.TON = 1;	// Start the timer.
	T4CONbits.TON = 1;	// Start the timer.

	TMR3 = 0;  	// Timer3:Timer2 high word
	TMR5 = 0;	// Timer5:Timer4 high word

	TMR2 = 0;  	// Timer3:Timer2 low word
	TMR4 = 0;	// Timer5:Timer4 low word
}	

// Assuming a 30.000MHz clock, one tick is 1/117187.5 seconds.
void Delay(unsigned int time) {
	unsigned int temp;
	temp = TMR1;	
	while (TMR1 - temp < time) {
//		ClrWdt();
	}
}
void DelaySeconds(unsigned int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(58593);  // maximum Delay is 65534.. One tick is 1/117187.5 seconds.  Delay for half of that, twice.  That is 1 second.
		Delay(58594);  
	}
}
void DelayTenthsSecond(unsigned int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(11719);  // 117187.5 ticks in Delay is 1 second.  So, 1/10 of that.
	}
}

void InitCNModule() {
	CNEN1bits.CN0IE = 1;  // overcurrent fault.
	CNEN1bits.CN1IE = 1;  // desat fault.

	CNPU1bits.CN0PUE = 0; // Make sure internal pull-up is turned OFF on CN0.
	CNPU1bits.CN1PUE = 0; // Make sure internal pull-up is turned OFF on CN1.

	_CNIF = 0;  // Clear change notification interrupt flag just to make sure it starts cleared.
	_CNIP = 3;  // Set the priority level for interrupts to 3.
	_CNIE = 1;  // Make sure interrupts are enabled.
}

void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void) {
	IFS0bits.CNIF = 0;  // clear the interrupt flag.

	if (I_PORT_DESAT_FAULT == 0) {  // It just became 0 like 2Tcy's ago.
		faultBits |= DESAT_FAULT;
	}
	if (I_PORT_OVERCURRENT_FAULT == 0) {
		faultBits |= OVERCURRENT_FAULT;
	}
}

void InitIORegisters() {
	I_TRIS_THROTTLE = 1;		// 1 means configure as input.  A/D throttle.
	I_TRIS_CURRENT1	= 1;		// 1 means configure as input.  A/D current 1.
	I_TRIS_CURRENT2	= 1;		// 1 means configure as input.  A/D current 2.
	I_TRIS_INDEX	= 1;		// 1 means configure as input.  encoder index.  1 pulse per revolution.
	I_TRIS_QEA		= 1;		// 1 means configure as input.  encoder QEA
	I_TRIS_QEB		= 1;		// 1 means configure as input.  encoder QEB
	O_TRIS_CLEAR_FLIP_FLOP = 0;	// 0 means configure as output. clear flip flop.
	O_LAT_CLEAR_FLIP_FLOP = 1;  // bring LOW, then HIGH to clear the flip flop.

	I_TRIS_TEMPERATURE = 1;	// 1 means configure as input.  A/D temperature.
	I_TRIS_REGEN_THROTTLE = 1;	// 1 means configure as input.  A/D regen throttle.
	I_TRIS_DESAT_FAULT = 1;		// 1 means configure as input.  desat fault.
	I_TRIS_OVERCURRENT_FAULT = 1;	// 1 means configure as input. overcurrent fault.
	I_TRIS_UNDERVOLTAGE_FAULT = 1;	// 1 means configure as input.  undervoltage fault.
	O_TRIS_LED = 0; 			// 0 means configure as output.  Status LED.
	O_LAT_LED = 1; 				 // high means turn ON the LED.
	O_TRIS_PRECHARGE_RELAY = 0;	// 0 means configure as output.  precharge relay control.
	O_LAT_PRECHARGE_RELAY = 0;	// HIGH means turn ON the precharge relay.
	I_TRIS_GLOBAL_FAULT	= 1;	
	O_TRIS_CONTACTOR = 0;		// 
	O_LAT_CONTACTOR = 0;	
	
	O_TRIS_CLEAR_DESAT = 0;	
	O_LAT_CLEAR_DESAT = 1;		// Bring low then high to clear desat.
	
	O_TRIS_PWM_3H = 0;		// 0 means configure as output.
	O_LAT_PWM_3H = 0;		// Low means OFF.
	
	O_TRIS_PWM_3L = 0;		// 0 means configure as output.
	O_LAT_PWM_3L = 0;		// Low means OFF.
			
	O_TRIS_PWM_2H = 0;		// 0 means configure as output.
	O_LAT_PWM_2H = 0;		// Low means OFF.
		
	O_TRIS_PWM_2L = 0;		// 0 means configure as output.
	O_LAT_PWM_2L = 0;		// Low means OFF.
	
	O_TRIS_PWM_1H = 0;		// 0 means configure as output.
	O_LAT_PWM_1H = 0;		// Low means OFF.
	
	O_TRIS_PWM_1L = 0;		// 0 means configure as output.
	O_LAT_PWM_1L = 0;		// Low means OFF.
	
	ADPCFG = 0b1111111001111000;  // 0 is analog.  1 is digital.
}

void InitDiscreteADConversions() {
	// ============= ADC - Measure 
	// ADC setup for simultanous sampling
	// AN0 = CH1 = Throttle
	// AN1 = CH2 = current1
	// AN2 = CH3 = current2
	// AN7 = CH0 = temperature

	// Note:  F.R.M. means Family Resource Manual for the dsPIC30F family of microcontrollers.
   
    ADCON1bits.FORM = 0;  // integer in the range 0-1023
    ADCON1bits.SSRC = 0;  // Clearing ADCON1bits.SAMP bit ends sampling and starts conversion.

    // Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
    // Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x)
    // Samples CH0 and CH1 simultaneously (when CHPS = 01)
    ADCON1bits.SIMSAM = 1; 

    // Sampling begins immediately after last conversion completes. 
    // SAMP bit is auto set.
	// ADCON1bits.ASAM = 1;  
    ADCON1bits.ASAM = 0;

/*
ADPCFG = 0xFFFB; // all PORTB = Digital; RB2 = analog
ADCON1 = 0x0000; // SAMP bit = 0 ends sampling ...
// and starts converting
ADCHS = 0x0002; // Connect RB2/AN2 as CH0 input ..
// in this example RB2/AN2 is the input
ADCSSL = 0;
ADCON3 = 0x0002; // Manual Sample, Tad = internal 2 Tcy
ADCON2 = 0;
ADCON1bits.ADON = 1; // turn ADC ON
while (1) // repeat continuously
{
ADCON1bits.SAMP = 1; // start sampling ...
DelayNmSec(100); // for 100 mS
ADCON1bits.SAMP = 0; // start Converting
while (!ADCON1bits.DONE); // conversion done?
ADCValue = ADCBUF0; // yes then get ADC value
}
*/
    // Pg. 407 in F.R.M.
    // Samples CH0, CH1, CH2, CH3 simultaneously when CHPS = 1x
    ADCON2bits.CHPS = 0b10; // VCFG = 000; This selects the A/D High voltage as AVdd, and A/D Low voltage as AVss.
						 // SMPI = 0000; This makes an interrupt happen every time the A/D conversion process is done (for all 4 I guess, since they happen at the same time.)
						 // ALTS = 0; Always use MUX A input multiplexer settings
						 // BUFM = 0; Buffer configured as one 16-word buffer ADCBUF(15...0)

    // Pg. 408 in F.R.M.
    // A/D Conversion Clock Select bits = 4 * Tcy.  (7+1) * Tcy/2 = 4*Tcy.
	// The A/D conversion of 4 simultaneous conversions takes 4*12*A/D clock cycles.  The A/D clock is selected to be 32*Tcy.
    // So, it takes about 96us to complete if ADCS = 63.
	ADCON3bits.ADCS = 31;  // 

    // ADCHS: ADC Input Channel Select Register 
    // Pg. 409 in F.R.M.
    // CH0 positive input is AN7, which is temperature.
    ADCHSbits.CH0SA = 7;
    	
	// CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2.
	ADCHSbits.CH123SA = 0;

	// CH0 negative input is Vref-.
	ADCHSbits.CH0NA = 0;

	// CH1, CH2, CH3 negative inputs are Vref-, which is AVss, which is Vss.  haha.
    ADCHSbits.CH123NA = 0;

    // Turn on A/D module
    ADCON1bits.ADON = 1; // Pg. 416 in F.R.M.  Under "Enabling the Module"
						 // ** It's important to set all the bits above before turning on the A/D module. **
						 // Now the A/D conversions start happening once ADON == 1.
	IEC0bits.ADIE = 0;	 // DISABLE interrupts for when an A/D conversion is complete. Pg. 148 of F.R.M.  
	return;
}

void InitADAndPWM() {
    ADCON1bits.ADON = 0; // Pg. 416 in F.R.M.  Under "Enabling the Module".  Turn it off for a moment.

	// PWM Initialization

	PTPER = 1499;	// 8KHz assuming 30MHz clock. 30,000,000/((PTPER + 1)*2) = 10KHz.
	PDC1 = 0;
	PDC2 = 0;
	PDC3 = 0;

	PWMCON1 = 0b0000000000000000; 			// Pg. 339 in FRM.
	PWMCON1bits.PEN1H = 1; // enabled. Enables pwm1h.
	PWMCON1bits.PEN1L = 1; // enabled. Enables pwmll.
	PWMCON1bits.PEN2H = 1; // enabled.
	PWMCON1bits.PEN2L = 1; // enabled.
	PWMCON1bits.PEN3H = 1; // enabled.
	PWMCON1bits.PEN3L = 1; // enabled. 
	
	PWMCON2 = 0;

    DTCON1 = 0;     		// Pg. 341 in Family Reference Manual. 
	DTCON1bits.DTAPS = 0b10;  // CLOCK Period is 4*Tcy.
	DTCON1bits.DTA = 30;  	// 30 CLOCK periods.  That is 4*30*Tcy = 4uS of dead time.

//    FLTACON = 0b0000000010000001; // Pg. 343 in Family Reference Manual. The Fault A input pin functions in the cycle-by-cycle mode.

	PTCON = 0x8002;         // Pg. 337 in FRM.  Enable PWM for center aligned operation.
							// PTCON = 0b1000000000000010;  Pg. 337 in Family Reference Manual.
							// PTEN = 1; PWM time base is ON
							// PTSIDL = 0; PWM time base runs in CPU Idle mode
							// PTOPS = 0000; 1:1 Postscale
							// PTCKPS = 00; PWM time base input clock period is TCY (1:1 prescale)
							// PTMOD = 10; PWM time base operates in a continuous up/down counting mode

	// SEVTCMP: Special Event Compare Count Register 
    // Phase of ADC capture set relative to PWM cycle: 0 offset and counting up
    SEVTCMP = 2;        // Cannot be 0 -> turns off trigger (Missing from doc)
						// SEVTCMP = 0b0000000000000010;  Pg. 339 in Family Reference Manual.
						// SEVTDIR = 0; A special event trigger will occur when the PWM time base is counting upwards and...
						// SEVTCMP = 000000000000010; If SEVTCMP == PTMR<14:0>, then a special event trigger happens.


	// ============= ADC - Measure 
	// ADC setup for simultanous sampling
	// AN0 = CH1 = Throttle;
	// AN1 = CH2 = Current1;
	// AN2 = CH3 = Current2;
	// AN8 = CH0 = regen throttle;	// trade between these 2
	// AN7 = CH0 = Temperature;		// trade between these 2.

	ADCON1 = 0;  // Starts this way anyway.  But just to be sure.   

    ADCON1bits.FORM = 0;  // unsigned integer in the range 0-1023
    ADCON1bits.SSRC = 0b011;  // Motor Control PWM interval ends sampling and starts conversion

    // Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
    // Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x)
    // Samples CH0 and CH1 simultaneously (when CHPS = 01)
    ADCON1bits.SIMSAM = 1; 
 
    // Sampling begins immediately after last conversion completes. 
    // SAMP bit is auto set.
    ADCON1bits.ASAM = 1;  

	ADCON2 = 0; // Pg. 407 in F.R.M.
    // Pg. 407 in F.R.M.
    // Samples CH0, CH1, CH2, CH3 simultaneously when CHPS = 1x
    ADCON2bits.CHPS = 0b10; // VCFG = 000; This selects the A/D High voltage as AVdd, and A/D Low voltage as AVss.
						 // SMPI = 0000; This makes an interrupt happen every time the A/D conversion process is done (for all 4 I guess, since they happen at the same time.)
						 // ALTS = 0; Always use MUX A input multiplexer settings
						 // BUFM = 0; Buffer configured as one 16-word buffer ADCBUF(15...0)


 	ADCON3 = 0; // Pg. 408 in F.R.M.
    // Pg. 408 in F.R.M.
    // A/D Conversion Clock Select bits = 16 * Tcy.  (31+1) * Tcy/2 = 16*Tcy.
	// The A/D conversion of 4 simultaneous conversions takes 4*12*A/D clock cycles.  The A/D clock is selected to be 4*Tcy.
    // So, it takes about 4*12*16*Tcy to complete 4 A/D conversions. That's 25.6uS. The pwm period is 125uS, since it's 8kHz nominal.
	ADCON3bits.ADCS = 31;  // 16Tcy.


    // ADCHS: ADC Input Channel Select Register 
    ADCHS = 0; // Pg. 409 in F.R.M.

    // ADCHS: ADC Input Channel Select Register 
    // Pg. 409 in F.R.M.
    // CH0 positive input is AN7, temperature.
    ADCHSbits.CH0SA = 7;
    	
	// CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2.
	ADCHSbits.CH123SA = 0;

	// CH0 negative input is Vref-.
	ADCHSbits.CH0NA = 0;

	// CH1, CH2, CH3 negative inputs are Vref-, which is AVss, which is Vss.  haha.
    ADCHSbits.CH123NA = 0;

    // ADCSSL: ADC Input Scan Select Register 
    ADCSSL = 0; // Pg. 410 F.R.M.
				// I think it sets the order that the A/D inputs are done.  But I'm doing 4 all at the same time, so set it to 0?


    // Turn on A/D module
    ADCON1bits.ADON = 1; // Pg. 416 in F.R.M.  Under "Enabling the Module"
						 // ** It's important to set all the bits above before turning on the A/D module. **
						 // Now the A/D conversions start happening once ADON == 1.
	_ADIP = 4;			 // A/D interrupt priority set to 4.  Default is 4.
	IEC0bits.ADIE = 1;	 // Enable interrupts to happen when a A/D conversion is complete. Pg. 148 of F.R.M.  	

	// QEICON starts as all zeros.
	// The Quadrature Encoder Interface.  This is for enabling the encoder.
	QEICONbits.QEIM = 0b111; 	// enable QEI x4 mode with position counter reset by MAXCNT.  
	QEICONbits.PCDOUT = 1; 	 	// Position Counter Direction Status Output Enable (QEI logic controls state of I/O pin)
	QEICONbits.POSRES = 0;		// Position counter is not reset by index pulse. But there's no index pulse in my case.  haha. This is ignored in this situation.
	QEICONbits.SWPAB = 0;		// don't swap QEA and QEB inputs.

	DFLTCONbits.CEID = 1; 		// Interrupts due to position count errors disabled
	DFLTCONbits.QEOUT = 1; 		// Digital filter outputs enabled.  QEA, QEB. 0 means normal pin operation.
	DFLTCONbits.QECK = 0b011; 	// clock prescaler of 16. So, QEA or QEB must be high or low for a time of 16*3 Tcy's. 
								// Fcy = 30MHz.  3*16Tcy is the minimum pulse width required for QEA or QEB to change from high to low or low to high.
								// You can do at most 30,000,000/(3*16) = 625,000 of those pulses per second.  So, you can have at most
								// 625,000 * 4 clock counts per second.  That means the maximum detectable RPM is:
								// x rev/sec * 2096 clockCounts/Rev = 625,000 * 4 clockCounts/sec.
								// So, x = 1193 rev/sec = 71564 RPM.  You could do with a prescaler of 64, but then there would be a lag of around 2.4 ticks of the 2048 degrees at 12000rpm
								// just from waiting around for the filter to accept a high or low signal.  I might change it later if noise is an issue.
								// 
	DFLTCONbits.IMV = 0b00;  	// INDEX pulse happens when QEA is low.  Irrelevant.

	MAXCNT = 0xFFFF; 	// reset the counter each time it reaches maxcnt.  It will reset anyway won't it?  I mean, after 0xFFFF is 0x0000!  haha.
						// Use this for the AC controller.  It's easier to measure speed of rotor with this setting, if there's no INDEX signal on the encoder.
	POSCNT=0;  // How many ticks have gone by so far?  Starts out as zero anyway.  It's safe to write to it though.

	PDC1 = 0;
	PDC2 = 0;
	PDC3 = 0;
}

// I know I should disable interrupts here, but I don't want to affect interrupt timing.
void FetchRTData() {
	RTData.throttleRaw = throttle;
	RTData.ADCurrent1 = ADCurrent1;
	RTData.ADCurrent2 = ADCurrent2;
	RTData.raw_hs_temp = temperature;
	RTData.Id = Id + 4096;
	RTData.IqRef = IqRef + 4096;  // range is [-4096, 4096], so shift to [0,8192] so as to avoid negative sign above.
	RTData.Iq = Iq + 4096;
	RTData.pdc1 = PDC1;
	RTData.pdc2 = PDC2;
	RTData.pdc3 = PDC3;
//	RTData.RPM = ((RPS_times64 * 15) >> 2) + 8000;  // RPS_TIMES16 * 60 / 16 = RPM.  Add 8000 so that it is guaranteed to be positive.  subtract later in Excel.
//	RTData.batteryAmps = batteryAmps + 4096;
	RTData.faultBits = faultBits;	
}

/*
void EESaveValues() {  // save the new stuff.
	EEDataInRamCopy1[0] = savedValues.Kp;		// PI loop proportional gain
	EEDataInRamCopy1[1] = savedValues.Ki;								// PI loop integreal gain
	EEDataInRamCopy1[2] = savedValues.throttleLowVoltage;		// throttle low voltage (foot off pedal)
	EEDataInRamCopy1[3] = savedValues.throttleHighVoltage;		// throttle high voltage (full throttle)
	EEDataInRamCopy1[4] = savedValues.throttleFaultVoltage;		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
	EEDataInRamCopy1[5] = savedValues.throttlePositionGain;		// gain for actual throttle position
	EEDataInRamCopy1[6] = savedValues.throttlePWMGain;			// gain for pwm (voltage)
	EEDataInRamCopy1[7] = savedValues.currentRampRate;				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
	EEDataInRamCopy1[8] = savedValues.datastreamPeriod;			// real time data period
	EEDataInRamCopy1[9] = savedValues.motorOverspeedThreshold;	// motor overspeed threshold
	EEDataInRamCopy1[10] = savedValues.motorOverspeedOffTime;	// motor overspeed fault time, in units of about 1/128 sec.
	EEDataInRamCopy1[11] = savedValues.maxBatteryAmperes;		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
	EEDataInRamCopy1[12] = savedValues.prechargeTime;			// precharge time in 0.1 second increments
	EEDataInRamCopy1[13] = savedValues.minAmperesForOverspeed;	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
	EEDataInRamCopy1[14] = savedValues.maxMotorAmperes;		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
	EEDataInRamCopy1[15] =  savedValues.Kp + savedValues.Ki + savedValues.throttleLowVoltage + savedValues.throttleHighVoltage + 
					  		savedValues.throttleFaultVoltage + savedValues.throttlePositionGain + savedValues.throttlePWMGain + 
					  		savedValues.currentRampRate + savedValues.datastreamPeriod + savedValues.motorOverspeedThreshold + savedValues.motorOverspeedOffTime +
					  		savedValues.maxBatteryAmperes + savedValues.prechargeTime + savedValues.minAmperesForOverspeed + savedValues.maxMotorAmperes;

    _erase_eedata(EE_addr_Copy1, _EE_ROW);
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr_Copy2, _EE_ROW);
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr_Copy3, _EE_ROW);
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr_Copy4, _EE_ROW);
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRamCopy1"
    _write_eedata_row(EE_addr_Copy1, EEDataInRamCopy1);
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRamCopy1"
    _write_eedata_row(EE_addr_Copy2, EEDataInRamCopy1);
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRamCopy1"
    _write_eedata_row(EE_addr_Copy3, EEDataInRamCopy1);
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRamCopy1"
    _write_eedata_row(EE_addr_Copy4, EEDataInRamCopy1);
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
	// 4 copies of the same thing, to be more robust.
}

void MoveDataFromEEPromToRAM() {
	int i = 0;
	unsigned int CRC1 = 0, CRC2 = 0, CRC3 = 0, CRC4 = 0;

	_memcpy_p2d16(EEDataInRamCopy1, EE_addr_Copy1, _EE_ROW);
	_memcpy_p2d16(EEDataInRamCopy2, EE_addr_Copy2, _EE_ROW);
	_memcpy_p2d16(EEDataInRamCopy3, EE_addr_Copy3, _EE_ROW);
	_memcpy_p2d16(EEDataInRamCopy4, EE_addr_Copy4, _EE_ROW);
	for (i = 0; i < 15; i++) { //for (i = 0; i < (sizeof(SavedValuesStruct) >> 1) - 1; i++) {  // i = 0; i < 15; i++.  Skip the last one, which is CRC.
		CRC1 += EEDataInRamCopy1[i];
		CRC2 += EEDataInRamCopy2[i];
		CRC3 += EEDataInRamCopy3[i];
		CRC4 += EEDataInRamCopy4[i];		
	}

	if (EEDataInRamCopy1[15] == CRC1) {  // crc from EEProm is OK for copy 1.  There has been a previously saved configuration.  
										// No need to load the default configuration. 
		savedValues.Kp = EEDataInRamCopy1[0];		// PI loop proportional gain
		savedValues.Ki = EEDataInRamCopy1[1];						// PI loop integral gain
		savedValues.throttleLowVoltage = EEDataInRamCopy1[2];		// throttle low voltage (pedal to metal)
		savedValues.throttleHighVoltage = EEDataInRamCopy1[3];		// throttle high voltage (foot off pedal)
		savedValues.throttleFaultVoltage = EEDataInRamCopy1[4];		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
		savedValues.throttlePositionGain = EEDataInRamCopy1[5];		// gain for actual throttle position
		savedValues.throttlePWMGain = EEDataInRamCopy1[6];			// gain for pwm (voltage)
		savedValues.currentRampRate = EEDataInRamCopy1[7];				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
		savedValues.datastreamPeriod = EEDataInRamCopy1[8];			// real time data period
		savedValues.motorOverspeedThreshold = EEDataInRamCopy1[9];	// motor overspeed threshold
		savedValues.motorOverspeedOffTime = EEDataInRamCopy1[10];	// motor overspeed fault time, in units of about 1/128 sec.
		savedValues.maxBatteryAmperes = EEDataInRamCopy1[11];		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
		savedValues.prechargeTime = EEDataInRamCopy1[12];			// precharge time in 0.1 second increments
		savedValues.minAmperesForOverspeed = EEDataInRamCopy1[13];	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
		savedValues.maxMotorAmperes = EEDataInRamCopy1[14];		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
		savedValues.crc = CRC1;
	}
	else if (EEDataInRamCopy2[15] == CRC2) {
		savedValues.Kp = EEDataInRamCopy2[0];		// PI loop proportional gain
		savedValues.Ki = EEDataInRamCopy2[1];								// PI loop integreal gain
		savedValues.throttleLowVoltage = EEDataInRamCopy2[2];		// throttle low voltage (pedal to metal)
		savedValues.throttleHighVoltage = EEDataInRamCopy2[3];		// throttle high voltage (foot off pedal)
		savedValues.throttleFaultVoltage = EEDataInRamCopy2[4];		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
		savedValues.throttlePositionGain = EEDataInRamCopy2[5];		// gain for actual throttle position
		savedValues.throttlePWMGain = EEDataInRamCopy2[6];			// gain for pwm (voltage)
		savedValues.currentRampRate = EEDataInRamCopy2[7];				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
		savedValues.datastreamPeriod = EEDataInRamCopy2[8];			// real time data period
		savedValues.motorOverspeedThreshold = EEDataInRamCopy2[9];	// motor overspeed threshold
		savedValues.motorOverspeedOffTime = EEDataInRamCopy2[10];	// motor overspeed fault time, in units of about 1/128 sec.
		savedValues.maxBatteryAmperes = EEDataInRamCopy2[11];		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
		savedValues.prechargeTime = EEDataInRamCopy2[12];			// precharge time in 0.1 second increments
		savedValues.minAmperesForOverspeed = EEDataInRamCopy2[13];	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
		savedValues.maxMotorAmperes = EEDataInRamCopy2[14];		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
		savedValues.crc = CRC2;
	}
	else if (EEDataInRamCopy3[15] == CRC3) {
		savedValues.Kp = EEDataInRamCopy3[0];		// PI loop proportional gain
		savedValues.Ki = EEDataInRamCopy3[1];								// PI loop integreal gain
		savedValues.throttleLowVoltage = EEDataInRamCopy3[2];		// throttle low voltage (pedal to metal)
		savedValues.throttleHighVoltage = EEDataInRamCopy3[3];		// throttle high voltage (foot off pedal)
		savedValues.throttleFaultVoltage = EEDataInRamCopy3[4];		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
		savedValues.throttlePositionGain = EEDataInRamCopy3[5];		// gain for actual throttle position
		savedValues.throttlePWMGain = EEDataInRamCopy3[6];			// gain for pwm (voltage)
		savedValues.currentRampRate = EEDataInRamCopy3[7];				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
		savedValues.datastreamPeriod = EEDataInRamCopy3[8];			// real time data period
		savedValues.motorOverspeedThreshold = EEDataInRamCopy3[9];	// motor overspeed threshold
		savedValues.motorOverspeedOffTime = EEDataInRamCopy3[10];	// motor overspeed fault time, in units of about 1/128 sec.
		savedValues.maxBatteryAmperes = EEDataInRamCopy3[11];		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
		savedValues.prechargeTime = EEDataInRamCopy3[12];			// precharge time in 0.1 second increments
		savedValues.minAmperesForOverspeed = EEDataInRamCopy3[13];	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
		savedValues.maxMotorAmperes = EEDataInRamCopy3[14];		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
		savedValues.crc = CRC3;
	}
	else if (EEDataInRamCopy4[15] == CRC4) {
		savedValues.Kp = EEDataInRamCopy4[0];		// PI loop proportional gain
		savedValues.Ki = EEDataInRamCopy4[1];								// PI loop integreal gain
		savedValues.throttleLowVoltage = EEDataInRamCopy4[2];		// throttle low voltage (pedal to metal)
		savedValues.throttleHighVoltage = EEDataInRamCopy4[3];		// throttle high voltage (foot off pedal)
		savedValues.throttleFaultVoltage = EEDataInRamCopy4[4];		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
		savedValues.throttlePositionGain = EEDataInRamCopy4[5];		// gain for actual throttle position
		savedValues.throttlePWMGain = EEDataInRamCopy4[6];			// gain for pwm (voltage)
		savedValues.currentRampRate = EEDataInRamCopy4[7];				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
		savedValues.datastreamPeriod = EEDataInRamCopy4[8];			// real time data period
		savedValues.motorOverspeedThreshold = EEDataInRamCopy4[9];	// motor overspeed threshold
		savedValues.motorOverspeedOffTime = EEDataInRamCopy4[10];	// motor overspeed fault time, in units of about 1/128 sec.
		savedValues.maxBatteryAmperes = EEDataInRamCopy4[11];		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
		savedValues.prechargeTime = EEDataInRamCopy4[12];			// precharge time in 0.1 second increments
		savedValues.minAmperesForOverspeed = EEDataInRamCopy4[13];	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
		savedValues.maxMotorAmperes = EEDataInRamCopy4[14];		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
		savedValues.crc = CRC4;
	}	
	else {	// There wasn't a single good copy.  Load the default configuration.
		savedValues.Kp = savedValuesDefault.Kp;		// PI loop proportional gain
		savedValues.Ki = savedValuesDefault.Ki;								// PI loop integreal gain
		savedValues.throttleLowVoltage = savedValuesDefault.throttleLowVoltage;		// throttle low voltage (pedal to metal)
		savedValues.throttleHighVoltage = savedValuesDefault.throttleHighVoltage;		// throttle high voltage (foot off pedal)
		savedValues.throttleFaultVoltage = savedValuesDefault.throttleFaultVoltage;		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
		savedValues.throttlePositionGain = savedValuesDefault.throttlePositionGain;		// gain for actual throttle position
		savedValues.throttlePWMGain = savedValuesDefault.throttlePWMGain;			// gain for pwm (voltage)
		savedValues.currentRampRate = savedValuesDefault.currentRampRate;				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
		savedValues.datastreamPeriod = savedValuesDefault.datastreamPeriod;			// real time data period
		savedValues.motorOverspeedThreshold = savedValuesDefault.motorOverspeedThreshold;	// motor overspeed threshold
		savedValues.motorOverspeedOffTime = savedValuesDefault.motorOverspeedOffTime;	// motor overspeed fault time, in units of about 1/128 sec.
		savedValues.maxBatteryAmperes = savedValuesDefault.maxBatteryAmperes;		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
		savedValues.prechargeTime = savedValuesDefault.prechargeTime;			// precharge time in 0.1 second increments
		savedValues.minAmperesForOverspeed = savedValuesDefault.minAmperesForOverspeed;	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
		savedValues.maxMotorAmperes = savedValuesDefault.maxMotorAmperes;		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps.
		savedValues.crc = savedValuesDefault.Kp + savedValuesDefault.Ki + savedValuesDefault.throttleLowVoltage + savedValuesDefault.throttleHighVoltage + 
						  savedValuesDefault.throttleFaultVoltage + savedValuesDefault.throttlePositionGain + savedValuesDefault.throttlePWMGain + 
						  savedValuesDefault.currentRampRate + savedValuesDefault.datastreamPeriod + savedValuesDefault.motorOverspeedThreshold + savedValuesDefault.motorOverspeedOffTime +
						  savedValuesDefault.maxBatteryAmperes + savedValuesDefault.prechargeTime + savedValuesDefault.minAmperesForOverspeed + savedValuesDefault.maxMotorAmperes;
	}

}
*/
