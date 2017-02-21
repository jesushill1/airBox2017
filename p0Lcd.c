// Include Files
//---------------------------
#include "p0Lcd.h"

// Externals
//---------------------------

// Constants
//---------------------------
#define		COM0		_pa1
#define		COM1		_pa4
#define		COM2		_pa0
#define		COM3		_pa2
#define		SEG0		_pb0
#define		SEG1		_pb1
#define		SEG2		_pb2
#define		SEG3		_pb3
#define		SEG4		_pb4
#define		SEG5		_pb5
#define		seg0	_pb0
#define		seg1	_pb1
#define		seg2	_pb2
#define		seg3	_pb3
#define		seg4	_pb4
#define		seg5	_pb5

#define		com0	_pa1
#define		com1	_pa4
#define		com2	_pa0
#define		com3	_pa2
#define		LIGHT		_pc2

#define SegA 0x80	/* COM3 + SEG1 */
#define SegB 0x40	/* COM2 + SEG1 */
#define SegC 0x20	/* COM1 + SEG1 */
#define SegD 0x10	/* COM0 + SEG1 */
#define SegE 0x01	/* COM0 + SEG0 */
#define SegF 0x04	/* COM2 + SEG0 */
#define SegG 0x02	/* COM1 + SEG0 */
#define SegS 0x08	/* COM3 + SEG0 */

uint8 test0, test1,test2,test3,test4,test5;

const uint8 DigSegTable[11] =
{
	SegA + SegB + SegC + SegD + SegE + SegF,		// '0'
	SegB + SegC,									// '1'
	SegA + SegB + SegD + SegE + SegG,				// '2'
	SegA + SegB + SegC + SegD + SegG,				// '3'
	SegB + SegC + SegF + SegG,						// '4'
	SegA + SegC + SegD + SegF + SegG,				// '5'
	SegA + SegC + SegD + SegE + SegF + SegG,		// '6'
	SegA + SegB + SegC,								// '7'
	SegA + SegB + SegC + SegD + SegE + SegF + SegG,	// '8'
	SegA + SegB + SegC + SegD + SegF + SegG,		// '9'
	SegG											// '-'
};

// Types and Classes
//---------------------------

// Function Prototypes
//---------------------------


// Public Variables
//---------------------------

// Private Variables
//---------------------------
uint16 LcdRefreshTimer;
uint8 LcdRefreshIndx;
uint8 LcdSegData[3];
uint16 LcdData;

void LcdInit(void)
{
	_slcdc0=0b00010011;		//工作电流100uA,SCOM0~SCOM3使能
	_slcdc1=0b00111111;		//SSEG0~SSEG5使能
	_slcdc2=0x00;
	_slcdc3=0x00;
	
	LcdRefreshIndx = 0;
	
	LcdTest();
}

void LcdTest(void)
{
	_frame=1;
	com0=1;com1=0;com2=0;com3=0;seg0=0;seg1=1;seg2=0;seg3=1;seg4=0;seg5=1;
}

void LcdUpdate(void)
{
	uint8 hundred, ten, digit;
	
	LcdSegData[0] = 0;
	LcdSegData[1] = 0;
	LcdSegData[2] = 0;
	
	switch(DataType)
	{
		case 1:			// Temperature
			LcdSegData[2] = SegS;
			LcdData = Temperature;		/* Temperautre is 2 digits */
			break;
		case 2:			// Humidity
			LcdSegData[1] = SegS;
			LcdData = Humidity;		/* Humidity is 2 digits */
			break;
		case 0: /* PM2.5 */
			LcdSegData[0] = SegS;
			LcdData = DustWeight;
			break;
		default:
			LcdData = 0;
	}
	if (LcdData < 0)
	{
		// If data is invalid, display "---"
		LcdSegData[0] += DigSegTable[10];
		LcdSegData[1] += DigSegTable[10];
		LcdSegData[2] += DigSegTable[10];
	}
	else
	{
		if (LcdData > 999)
		{
			hundred = 9;
			ten = 9;
			digit = 9;
		}
		else
		{
			hundred = LcdData / 100;
			ten = (LcdData % 100) / 10;
			digit = LcdData % 10;
		}
		if ((hundred != 0) || (DataType == 0))
		{
			LcdSegData[0] += DigSegTable[hundred];
		}
		LcdSegData[1] += DigSegTable[ten];
		LcdSegData[2] += DigSegTable[digit];
	}

}
void LcdRefresh()
{	
	LcdSegData[0]	= 0b10000000;
	LcdSegData[1]	= 0;
	LcdSegData[2]	= 0;

	LcdRefreshIndx++;
	if(LcdRefreshIndx>3)
	{
		LcdRefreshIndx = 0;
		_frame=!_frame;
	}
	// low/high active
	com0=0;com1=0;com2=0;com3=0;
	
	test0=(LcdSegData[0] & (0b00001000 >>LcdRefreshIndx))? 1 : 0;
	test1=(LcdSegData[0] & (0b10000000 >>LcdRefreshIndx))? 1 : 0;
	test2=(LcdSegData[1] & (0b00001000 >>LcdRefreshIndx))? 1 : 0;
	test3=(LcdSegData[1] & (0b10000000 >>LcdRefreshIndx))? 1 : 0;
	test4=(LcdSegData[2] & (0b00001000 >>LcdRefreshIndx))? 1 : 0;
	test5=(LcdSegData[2] & (0b10000000 >>LcdRefreshIndx))? 1 : 0;
	seg0=test0;
	seg1=test1;
	seg2=test2;
	seg3=test3;
	seg4=test4;
	seg5=test5;
	//seg0=0;seg1=1;seg2=0;seg3=1;seg4=0;seg5=1;
	
	switch(LcdRefreshIndx)
	{
		case 0:
			com0=1;
			break;
		case 1:
			com1=1;
			break;
		case 2:
			com2=1;
			break;
		case 3:
			com3=1;
			break;
		default:
			break;
	}

}

void LcdLightOn(void)
{
	LIGHT	= 1;
}
void LcdLightOff(void)
{
	LIGHT	= 0;
}
void LcdLightTog(void)
{
	LIGHT	= !LIGHT;
}