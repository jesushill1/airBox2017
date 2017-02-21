#ifndef P0LCD_H
#define P0LCD_H

// Include Files
//---------------------------
#include "Product.h"

// Externals
//---------------------------

// Constants
//---------------------------

// Types and Classes
//---------------------------

// Function Prototypes
//---------------------------
void LcdInit(void);
void LcdRefresh(void);
void LcdTest(void);
void LcdUpdate(void);
void LcdLightOn(void);
void LcdLightOff(void);
void LcdLightTog(void);

// Public Variables
//---------------------------
extern uint16 LcdRefreshTimer;

#endif
