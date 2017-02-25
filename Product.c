// Include Files
//---------------------------
#include "Product.h"
#include "p0Lcd.h"
#include "p0KeyProc.h"


// Externals
//---------------------------

// Constants
//---------------------------

// Types and Classes
//---------------------------

// Function Prototypes
//---------------------------
void __attribute((interrupt(0x0c))) isr_psc(void);

// Public Variables
//---------------------------
uint8 	State;		// AirBox State Variable:
					// 0: Idle state
					// 1: Initializtion state
					// 2: Normal state + Test state
					// 3: Pre-Idle state
uint8	LastState;	// Latched status before process
uint8	DataType;	// Data type to display on LCD:
					// 0: Temperature
					// 1: Humidity
					// 2: PM2.5 Weight
uint8	TestMode;	// 0: Normal mode (sleep after 10 seconds); 1: Test mode (don't sleep )
uint8	Temperature;
uint8	Humidity;
uint8	DustWeight;
uint8	flag;
uint16	rolated_speed;

void main()
{

	// Initialization
	
	//_pac=0b11101000;
	//_pbc=0b11000000;
	_pcc2=0;				// enable the background light output pin

	LcdInit();
	// System clock interruption
	_pscr	= 0b00000000;
	_tbc	= 0b00001111;
	_tb0e	= 1;
	// Enable global interruption
	_emi	= 1;
	
	LcdLightOn();
	// Infinite loop
	while (TRUE)
	{
		if(LongPress==0)
		{
			//TurnOnBackLight();
			if(ShortPress == 1)
			{
				ShortPress = 0;
				DataType = (DataType + 1) % 3;
				//LCD_Refresh(DataType, Temperature, Humidity, DustWeight);
			}
			if (LcdRefreshTimer == 0)
			{
				//BTNAME();
				//RefreshTimer = 2000;
				//ReadTH(&Temperature, &Humidity);
				//ReadRolatedSpeed(&rolated_speed);
				//if(rolated_speed == 0 || rolated_speed == 0xFFFF)
				//	goto close;
				//SetSpeed(rolated_speed);

				//ReadDustWeight(&DustWeight);
				//LCD_Refresh(DataType, Temperature, Humidity, DustWeight);
				//Send2BT(Temperature, Humidity, DustWeight);
			}
		}
		else //LongPress==1
		{
			if(1)
			{
				//TurnOffBackLight();
				//backLightOn = 0;
			}else
			{
				//TurnOnBackLight();
				//backLightOn = 1;
			}
			//LongPress =0;
		}
	} // end of while loop

}

void isr_psc()
{	
	// clear interrupt flag
	_tb0f	= 0;
	
	// reset watchdog
	_clrwdt();
	
	LcdRefresh();
	return;
}