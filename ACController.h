#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "p30F4011.h"
//#include "UART4011.h"
#include <libpic30.h>

#define I_TRIS_THROTTLE 		_TRISB0
#define I_TRIS_CURRENT1			_TRISB1
#define I_TRIS_CURRENT2			_TRISB2
#define I_TRIS_INDEX			_TRISB3
#define I_TRIS_QEA				_TRISB4
#define I_TRIS_QEB				_TRISB5
#define O_TRIS_CLEAR_FLIP_FLOP	_TRISB6
#define I_TRIS_TEMPERATURE		_TRISB7
#define I_TRIS_REGEN_THROTTLE	_TRISB8
#define I_TRIS_DESAT_FAULT		_TRISC13
#define I_TRIS_OVERCURRENT_FAULT 	_TRISC14
#define I_TRIS_UNDERVOLTAGE_FAULT	_TRISE8
#define O_TRIS_LED  			_TRISD1 // high means turn ON the LED.
#define O_TRIS_PRECHARGE_RELAY	_TRISD3	// HIGH means turn ON the precharge relay.
#define I_TRIS_GLOBAL_FAULT		_TRISD2
#define O_TRIS_CONTACTOR		_TRISD0
#define O_TRIS_CLEAR_DESAT		_TRISF6
#define O_TRIS_PWM_3H			_TRISE5
#define O_TRIS_PWM_3L			_TRISE4
#define O_TRIS_PWM_2H			_TRISE3
#define O_TRIS_PWM_2L			_TRISE2
#define O_TRIS_PWM_1H			_TRISE1
#define O_TRIS_PWM_1L			_TRISE0

#define I_LAT_THROTTLE 			_LATB0
#define I_LAT_CURRENT1			_LATB1
#define I_LAT_CURRENT2			_LATB2
#define I_LAT_INDEX				_LATB3
#define I_LAT_QEA				_LATB4
#define I_LAT_QEB				_LATB5
#define O_LAT_CLEAR_FLIP_FLOP	_LATB6
#define I_LAT_TEMPERATURE		_LATB7
#define I_LAT_REGEN_THROTTLE	_LATB8
#define I_LAT_DESAT_FAULT		_LATC13
#define I_LAT_OVERCURRENT_FAULT _LATC14
#define I_LAT_UNDERVOLTAGE_FAULT	_LATE8
#define O_LAT_LED  				_LATD1 // high means turn ON the LED.
#define O_LAT_PRECHARGE_RELAY	_LATD3	// HIGH means turn ON the precharge relay.
#define I_LAT_GLOBAL_FAULT		_LATD2
#define O_LAT_CONTACTOR			_LATD0
#define O_LAT_CLEAR_DESAT		_LATF6
#define O_LAT_PWM_3H			_LATE5
#define O_LAT_PWM_3L			_LATE4
#define O_LAT_PWM_2H			_LATE3
#define O_LAT_PWM_2L			_LATE2
#define O_LAT_PWM_1H			_LATE1
#define O_LAT_PWM_1L			_LATE0

#define I_PORT_THROTTLE 		_RB0
#define I_PORT_CURRENT1			_RB1
#define I_PORT_CURRENT2			_RB2
#define I_PORT_INDEX			_RB3
#define I_PORT_QEA				_RB4
#define I_PORT_QEB				_RB5
#define O_PORT_CLEAR_FLIP_FLOP	_RB6
#define I_PORT_TEMPERATURE		_RB7
#define I_PORT_REGEN_THROTTLE	_RB8
#define I_PORT_DESAT_FAULT		_RC13
#define I_PORT_OVERCURRENT_FAULT _RC14
#define I_PORT_UNDERVOLTAGE_FAULT	_RE8
#define O_PORT_LED  			_RD1 // high means turn ON the LED.
#define O_PORT_PRECHARGE_RELAY	_RD3	// HIGH means turn ON the precharge relay.
#define I_PORT_GLOBAL_FAULT		_RD2
#define O_PORT_CONTACTOR		_RD0
#define O_PORT_CLEAR_DESAT		_RF6
#define O_PORT_PWM_3H			_RE5
#define O_PORT_PWM_3L			_RE4
#define O_PORT_PWM_2H			_RE3
#define O_PORT_PWM_2L			_RE2
#define O_PORT_PWM_1H			_RE1
#define O_PORT_PWM_1L			_RE0


#define THROTTLE_FAULT (1 << 0)
#define VREF_FAULT (1 << 1)
#define UART_FAULT (1 << 2)
#define UV_FAULT (1 << 3)
#define OVERCURRENT_FAULT (1 << 4)

#define DESAT_FAULT (1 << 5)
#define MOTOR_OVERSPEED_FAULT (1 << 6)
#define ROTOR_FLUX_ANGLE_FAULT (1 << 7)

#define THROTTLE_REGEN_START 344
#define THROTTLE_REGEN_END 103
#define THROTTLE_START 474
#define THROTTLE_END 921

#define THROTTLE_FAULT_COUNTS 20		// after 100mS, flag a throttle fault.
#define MAX_AMPERES 
#define BAUD_RATE 19200
#define DELAY_200MS_SLOW_TIMER 23438  // 117.188KHz clock, 200ms.
#define THERMAL_CUTBACK_START 670	// 75degC
#define THERMAL_CUTBACK_END 726	// 85degC
// max temperature is 726, which is 85degC.

#define R_MAX 1727	// 1727 * 2/sqrt(3) * 1.5 = 2991.  Max duty will be 3000.  1.5 due to scaling of inverse clarke. 2/sqrt(3) I forgot now.  haha.
#define R_MAX_TIMES_65536 (1727L << 16)
#define STATOR_FIELD_CURRENT (32 << 4)  // assuming 50amp == 128 ticks of A/D, and power factor of 0.78.  a^2 + .78^2 = 1^2.  a = 0.625779.  So, field current (rms) is 12.8amp = 32.8 ticks of LEM 50.  Now, I'm scaling the current by 16, so << 4.

typedef struct {
	int Kp_Id;								// PI loop proportional gain
	int Ki_Id;								// PI loop integreal gain
	int Kp_Iq;
	int Ki_Iq;
	int maxRegen;		//
	int minRegen;		//
	int minThrottle;			//
	int maxThrottle;
	int throttleFault;	
	int maxBatteryAmps;			// battery amps limit.  Unit is amperes. 
	int maxBatteryRegenAmps;		// battery regen amp limit.  Unit is amperes.
	int prechargeTime;				// precharge time in 0.1 second increments
	unsigned crc;
} SavedValuesStruct;

typedef struct {
	int throttleRaw;
	int ADCurrent1;
	int ADCurrent2;
	unsigned raw_hs_temp;
	int IqRef;
	int Id;
	int Iq;
	unsigned pdc1;
	unsigned pdc2;
	unsigned pdc3;
	int RPM;
	unsigned batteryAmps; 
	unsigned faultBits;  
} realtime_data_type;

typedef struct {
	long K1;
	long K2;
	long error_new;
	long error_old;
	long pwm;
} piType;


#endif
