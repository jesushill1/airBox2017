#ifndef PRODUCT_H
#define PRODUCT_H

// Include Files
//---------------------------
#include <BS82C16A-3.H>

// Externals
//---------------------------

// Constants
//---------------------------
#define TRUE (1)
#define FALSE (0)

// Types and Classes
//---------------------------
typedef unsigned int uint8;
typedef unsigned long int uint16;
typedef int int8;
typedef long int int16;

// Function Prototypes
//---------------------------

// Public Variables
//---------------------------
extern uint8 	State;		// AirBox State Variable:
							// 0: Idle state
							// 1: Initializtion state
							// 2: Normal state + Test state
							// 3: Pre-Idle state
extern uint8	LastState;	// Latched status before process
extern uint8	DataType;	// Data type to display on LCD:
							// 0: Temperature
							// 1: Humidity
							// 2: PM2.5 Weight
extern uint8	TestMode;	// 0: Normal mode (sleep after 10 seconds); 1: Test mode (don't sleep )
extern uint8	Temperature;
extern uint8	Humidity;
extern uint8	DustWeight;
extern uint8	flag;
extern uint16	rolated_speed;

#endif
