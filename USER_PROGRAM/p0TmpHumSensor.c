#include    "USER_PROGRAM.H" 
#include    "p0TmpHumSensor.h"
#define Bit_RESET 0
#define Bit_SET 1
#define OK 1
#define ERROR 0
#define DHT11_TIMEOUT 400		/* 100 us time out constant */
#define     LIGHT        _pc2


inline unsigned char DHT11_PinValue()
{
    return _pc3;
}
static volatile unsigned char 	dht11_status 		__attribute__((at(0x3a0)));
static volatile unsigned char 	TmpHumReadErrCnt 	__attribute__((at(0x3a1)));

#define Delay_us(us)  GCC_DELAY(us*4)


static unsigned DHT11_ReadByte(void)
{
	unsigned char i, value = 0;
	unsigned int count;
	dht11_status = OK;
	for (i=0; i<8; i++)
	{
		value <<= 1;	/* MSB first */
		count = 0;		/* Every bit has a low period of 50us. */
		while ((DHT11_PinValue() == Bit_RESET) && (count++  < DHT11_TIMEOUT));
		if (count >= DHT11_TIMEOUT)
		{
			/* pin is pull low over time */
			dht11_status = ERROR;
			return 0;
		}
		
		/* 26-28us high level means '0', 70us high level means '1' */
		Delay_us(30);	/* Delay 30 us */
		if (DHT11_PinValue() == Bit_SET)
		{
			/* Means bit is '1' */
			value ++;
			/* Wait remining 40 us */
			count = 0;
			while ((DHT11_PinValue() == Bit_SET) && (count++  < DHT11_TIMEOUT));

			if (count >= DHT11_TIMEOUT)
			{
				/* pin is high over time */
				dht11_status = ERROR;
				return 0;
			}
		}
	}
	return (value);
}
void TmpHumRead(volatile int* pTemperature, volatile unsigned int * pHumidity)
{
	unsigned char i, check_value=0;
	unsigned char value_array[5];
	unsigned int count;
	
	/* Pull data low more than 18 ms */
	//DHT11_PinPullLow();
	//Delay_us(20000);
	//LIGHT=1;
	/* Release data line, wait 50us and check ACK (DHT11 pull low) */
	//DHT11_PinRelease();
	Delay_us(50);
	LIGHT=0;
	if (DHT11_PinValue() == Bit_SET)
	{
		/* no ACK, terminate read */
		//LIGHT=0;
		return;
	}
	else
	{
		/* get ACK */
		/* Wait until DHT11 release data line */
		count = 0;
		while ((DHT11_PinValue() == Bit_RESET) && (count++  < DHT11_TIMEOUT));
		if (count >= DHT11_TIMEOUT)
		{
			/* pin is low over time */
			//LIGHT=0;
			return;
		}
		/* Wait until DHT11 pull data line low for first bit*/
		count = 0;		
		while ((DHT11_PinValue() == Bit_SET) && (count++  < DHT11_TIMEOUT));
		if (count >= DHT11_TIMEOUT)
		{
			/* pin is low over time */
			//LIGHT=0;
			return;
		}
		
		/* Read temperature and humidity */
		for (i=0; i<5; i++)
		{
			//LIGHT=!LIGHT;
			value_array[i] = DHT11_ReadByte();
			if (dht11_status == ERROR)
			{
				//LIGHT=0;
				return;
			}
			if (i != 4)
			{
				check_value += value_array[i];
			}
		}
		if (check_value == value_array[4])
		{
			*pTemperature = value_array[2];
			*pHumidity 		= value_array[0];
			//LIGHT=0;
			return;
		}
		else
		{	/* checksum error */
			//LIGHT=0;
			return;
		}
	}	
}
