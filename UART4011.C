#include "UART4011.h"

void ShowMenu(void);
void ShowConfig(unsigned int mask);
void u16x_to_str(char *str, unsigned val, unsigned char digits);
void u16_to_str(char *str, unsigned val, unsigned char digits);
int TransmitString(char* str);
char IntToCharHex(unsigned int i);
void FetchRTData(void);
extern void InitPIStruct(void);
extern void NormalizeAllConfigurationCurrentsTo_0_512(void);
extern void EESaveValues(void);
UARTCommand myUARTCommand;
char commandText[32];
char tempStr[80];

unsigned int timeOfLastDatastreamTransmission = 0;
unsigned int commandNumber = 0;
unsigned int commandType = 0;	// 0 means no number argument.
extern unsigned int faultBits;
extern SavedValuesStruct savedValues;
extern unsigned long int motorOverspeedThresholdTimes1024;
extern unsigned int counter1ms;
extern unsigned int showDatastreamJustOnce;
extern unsigned int dataStreamPeriod;

char command0[] = "config";
char command1[] = "save";
char command2[] = "idle";
char command3[] = "restart";
char command4[] = "kp-id";
char command5[] = "ki-id";
char command6[] = "kp-iq";
char command7[] = "ki-iq";
								// throttle fault < max regen < min regen < min throttle < max throttle.  Wig Wag configuration.
char command8[] = "max-regen";  // max regen
char command9[] = "min-regen";  // min regen 
char command10[] = "min-throttle";  // min throttle
char command11[] = "max-throttle";  // max throttle
char command12[] = "throttle-fault"; // below this indicates throttle unplugged.

char command13[] = "rtd-period";
char command14[] = "rtd";
char command15[] = "max-battery-amps";
char command16[] = "max-battery-regen-amps";
char command17[] = "pc-time";

char savedEEString[] = "Configuration written to EE\r\n";
char menuString[] = "AC controller firmware, ver. 1.0\r\n";

char showConfigKpKiString[] = "Kp_Id=xxxx Ki_Id=xxxx Kp_Iq=xxxx Ki_Iq=xxxx\r\n";
char showConfigThrottleString[] = "max_regen=xxxx min_regen=xxxx min_throttle=xxxx max_throttle=xxxx throttle_fault=xxxx\r\n";
char showConfigBatteryAmpString[] = "max-battery-amps=xxxx max-battery-regen-amps=xxxx\r\n";  // range is [0amp,100amp] = [0,4096]????
char showConfigPrechargeString[] = "precharge_time=xxx\r\n";

void InitUART2() {
	U2BRG = 97; // Pg. 506 on F.R.M.  Baud rate is 19.2kbps. 30MHz
	U2MODE = 0;  // initialize to 0.
	U2MODEbits.PDSEL = 0b00; // 8 N 
	U2MODEbits.STSEL = 0; // 1 stop bit.

	IEC1bits.U2RXIE = 1;  // enable receive interrupts.
	IPC6bits.U2RXIP = 2;	// INTERRUPT priority of 2.
//bit 7-6 URXISEL<1:0>: Receive Interrupt Mode Selection bit
//11 =Interrupt flag bit is set when Receive Buffer is full (i.e., has 4 data characters)
//10 =Interrupt flag bit is set when Receive Buffer is 3/4 full (i.e., has 3 data characters)
//0x =Interrupt flag bit is set when a character is received
	U2STAbits.URXISEL = 0b00;  // 0b11 later..

	U2MODEbits.UARTEN = 1; // enable the uart
	asm("nop");
	U2STAbits.UTXEN = 1; // Enable transmissions
}

int TransmitReady() {
	if (U2STAbits.UTXBF == 1) // Pg. 502 in F.R.M.  Is transmit buffer full?
		return 0;
	return 1; 
}

void SendCharacter(char ch) {
	// Make sure to run TransmitReady() before this.
	U2TXREG = ch;
}
int ReceiveBufferHasData() {
	return U2STAbits.URXDA;  // returns 1 if true.  0 otherwise.
}

unsigned char GetCharacter() {
	return (unsigned char)U2RXREG;
}

void ClearReceiveBuffer() {
	U2STAbits.OERR = 0;  // clear the error.
}

void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void) {
	static unsigned char temp = 0;
/*
	IFS1bits.U2RXIF = 0;  // clear the interrupt.
	temp = U2RXREG;		// get the character that caused the interrupt.
	if (myUARTCommand.complete == 1) {	// just ignore everything until the command is processed.
		return;
	}
	if (temp == 0x0d) {	
		myUARTCommand.complete = 1;
		myUARTCommand.string[myUARTCommand.i] = 0;

		return;
	}
	myUARTCommand.string[myUARTCommand.i] = temp; // save the character that caused the interrupt!
	myUARTCommand.i++;
	if (myUARTCommand.i >= MAX_COMMAND_LENGTH) {  // the command was too long.  It's just garbage anyway, so start over.
		myUARTCommand.complete = 0;  // It can't make it here unless myUARTCommand.complete == 0 anyway.
		myUARTCommand.i = 0;	// just clear the array, and start over.
		myUARTCommand.string[0] = 0;
		return;
	}
*/
}

// process the command, and reset UARTCommandPtr back to zero.
// myUARTCommand is of the form XXXXXXXXX YYYYY CR
void ProcessCommand(void) {
	static int i = 0;
	static int w = 0;  // ?? what?!
/*
	if (myUARTCommand.complete == 0) {
		return;
	}
	commandNumber = 0;	// set number argument to zero.
	for (i = 0; myUARTCommand.string[i] != 0; i++) {
		if (myUARTCommand.string[i] == ' ') {
			commandText[i] = 0;  // null terminate the text portion.
			commandNumber = atoi(&myUARTCommand.string[i+1]);
			break;
		}
		commandText[i] = myUARTCommand.string[i];

	}
	commandText[i] = 0;  // NULL TERMINATE IT!!!

	if (myUARTCommand.i == 0) {	// The string was a carriage return.
		w = TransmitString(&menuString[0]);
		dataStreamPeriod = 0;  // stop the stream if it was going.
	}
	else if (!strcmp(commandText, command0)) { // "config"
		ShowConfig(0x0FFFF);
	}
	else if (!strcmp(&commandText[0], command1)){ // "save"
		TransmitString(savedEEString);  // 	"configuration written to EE"	
		EESaveValues();
	}
	else if (!strcmp(&commandText[0], command2)){ // "idle". 
	}
	else if (!strcmp(&commandText[0], command3)){ // "restart"
		while(1);	// this starts the program over, due to the watchdog.
	}
	else if (!strcmp(&commandText[0], command4)){ // "kp-id"
		if (commandNumber <= 9999) {
			savedValues.Kp_Id = commandNumber; InitPIStruct();
			ShowConfig((unsigned)1 << 0);
		}		
	}

//typedef struct {
//	unsigned Kp_Id;								// PI loop proportional gain
//	unsigned Ki_Id;								// PI loop integreal gain
//	unsigned Kp_Iq;
//	unsigned Ki_Iq;
//	unsigned maxRegen;		//
//	unsigned minRegen;		//
//	unsigned minThrottle;			//
//	unsigned maxThrottle;	
//	unsigned maxBatteryAmperes;			// battery amps limit.  Unit is amperes. 
//  unsigned maxBatteryRegenAmps;		// battery regen amp limit.  Unit is amperes.
//	unsigned prechargeTime;				// precharge time in 0.1 second increments
//	unsigned crc;
//} SavedValuesStruct;
	else if (!strcmp(&commandText[0], command5)){ // "ki-id"
		if (commandNumber <= 9999) {
			savedValues.Ki_Id = commandNumber; InitPIStruct();
			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp(&commandText[0], command6)){ // "kp-iq"
		if ((unsigned)commandNumber <= 9999) {
			savedValues.Kp_Iq = commandNumber; InitPIStruct();
			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp(&commandText[0], command6)){ // "ki-iq"
		if ((unsigned)commandNumber <= 9999) {
			savedValues.Ki_Iq = commandNumber; InitPIStruct();
			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp(&commandText[0], command7)){ // "max-regen"
		if ((unsigned)commandNumber <= 999) {
			savedValues.maxRegen = commandNumber; 
			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp(&commandText[0], command8)){ // "min-regen"
		if ((unsigned)commandNumber <= 999) {
			savedValues.minRegen = commandNumber;
			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp(&commandText[0], command9)){ // "min-throttle"
		if ((unsigned)commandNumber <= 999) {
			savedValues.minThrottle = commandNumber;
			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp(&commandText[0], command10)){ // "max-throttle"
		if ((unsigned)commandNumber <= 999) {
			savedValues.maxThrottle = commandNumber;
			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp(&commandText[0], command11)){ // "throttle-fault"
		if ((unsigned)commandNumber <= 999) {
			savedValues.throttleFault = commandNumber;
			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp(&commandText[0], command13)){ // "rtd-period"
		if ((unsigned)commandNumber <= 32000) {
			dataStreamPeriod = commandNumber;
			showDatastreamJustOnce = 0;
			timeOfLastDatastreamTransmission = counter1ms;	
			ShowConfig((unsigned)1 << 2);
		}
	}
	else if (!strcmp(&commandText[0], command14)){ // "rtd"
		dataStreamPeriod = 1;  // just make it nonzero.
		showDatastreamJustOnce = 1;
		ShowConfig((unsigned)1 << 6);
	}
	else if (!strcmp(&commandText[0], command15)){ // "max-battery-amps"
		if ((unsigned)commandNumber <= 9999) {  //  each phase current is in the range [-
			savedValues.maxBatteryAmps = commandNumber;
			ShowConfig((unsigned)1 << 10);
		}
	}
	else if (!strcmp(&commandText[0], command16)){ // "max-battery-regen-amps"
		if ((unsigned)commandNumber <= 9999) {  //  each phase current is in the range [-4096, 4096] if it's in [-100amp,100amp] for the LEM Hass 50-s
			savedValues.maxBatteryRegenAmps = commandNumber;
			ShowConfig((unsigned)1 << 10);
		}
	}
	else if (!strcmp(&commandText[0], command17)){ // "pc-time"
		if ((unsigned)commandNumber <= 999) {
			savedValues.prechargeTime = commandNumber;
			ShowConfig((unsigned)1 << 11);
		}
	}
	myUARTCommand.string[0] = 0; 	// clear the string.
	myUARTCommand.i = 0;
	myUARTCommand.complete = 0;  // You processed that command.  Dump it!  Do this last.  The ISR will do nothing as long as the command is complete.	
*/
}

int TransmitString(char* str) {  // For echoing onto the display
	static unsigned int i = 0;
	static unsigned int now = 0;
	
	now = TMR5;	// timer 4 runs at 117KHz.  Timer5 is the high word of the 32 bit timer.  So, it updates at around 0.5Hz.
	while (1) {
		if (str[i] == 0) {
			i = 0;
			break;
		}
		if (U2STAbits.UTXBF == 0) { // TransmitReady();
			U2TXREG = str[i]; 	// SendCharacter(str[i]);
			i++;
		}
//		if (TMR5 - now > 3) { 	// 2-3 seconds
//			faultBits |= UART_FAULT;
//			return 0;
//		}
		//		ClrWdt();
	}
	return 1;  
}

void ShowMenu(void)
{
	TransmitString(menuString);
}

// convert val to string (inside body of string) with specified number of digits
// do NOT terminate string
void u16_to_str(char *str, unsigned val, unsigned char digits)
{
	str = str + (digits - 1); // go from right to left.
	while (digits > 0) { // 
		*str = (unsigned char)(val % 10) + '0';
		val = val / 10;
		str--;
		digits--;
	}
}

// convert val to hex string (inside body of string) with specified number of digits
// do NOT terminate string
void u16x_to_str(char *str, unsigned val, unsigned char digits)
{
	unsigned char nibble;
	
	str = str + (digits - 1);
	while (digits > 0) {
		nibble = val & 0x000f;
		if (nibble >= 10) nibble = (nibble - 10) + 'A';
		else nibble = nibble + '0';
		*str = nibble;
		val = val >> 4;
		str--;
		digits--;
	}
}

void ShowConfig(unsigned int mask) {
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// Kp_Id=xxxx Ki_Id=xxxx Kp_Iq=xxxx Ki_Iq=xxxx
	if (mask & ((unsigned)1 << 0)) {
		u16_to_str(&showConfigKpKiString[6], savedValues.Kp_Id, 4);	
		u16_to_str(&showConfigKpKiString[17], savedValues.Ki_Id, 4);
		u16_to_str(&showConfigKpKiString[28], savedValues.Kp_Iq, 4);	
		u16_to_str(&showConfigKpKiString[39], savedValues.Ki_Iq, 4);
		TransmitString(showConfigKpKiString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// max_regen=xxx min_regen=xxx min_throttle=xxx max_throttle=xxx throttle_fault=xxx
	if (mask & ((unsigned)1 << 1)) {
		u16_to_str(&showConfigThrottleString[10], savedValues.maxRegen, 3);
		u16_to_str(&showConfigThrottleString[24], savedValues.minRegen, 3);
		u16_to_str(&showConfigThrottleString[41], savedValues.minThrottle, 3);
		u16_to_str(&showConfigThrottleString[58], savedValues.maxThrottle, 3);
		u16_to_str(&showConfigThrottleString[77], savedValues.throttleFault, 3);
		TransmitString(showConfigThrottleString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// max-battery-amps=xxxx max-battery-regen-amps=xxxx
	if (mask & ((unsigned)1 << 2)) {
		u16_to_str(&showConfigBatteryAmpString[17], savedValues.maxBatteryAmps, 4);
		u16_to_str(&showConfigBatteryAmpString[45], savedValues.maxBatteryRegenAmps, 4);
		TransmitString(showConfigBatteryAmpString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// precharge_time=xxx
	if (mask & ((unsigned)1 << 3)) {
		u16_to_str(&showConfigPrechargeString[15], savedValues.prechargeTime, 3);
		TransmitString(showConfigPrechargeString);
	}
}

// Input is an integer from 0 to 15.  Output is a character in '0', '1', '2', ..., '9', 'a','b','c','d','e','f'
char IntToCharHex(unsigned int i) {
	if (i <= 9) {
		return ((unsigned char)(i + 48));
	}
	else {
		return ((unsigned char)(i + 55));
	}
}

