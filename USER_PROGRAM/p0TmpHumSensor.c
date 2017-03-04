#include    "USER_PROGRAM.H" 
#define Bit_RESET 0
#define Bit_SET 1
#define OK 1
#define ERROR 0
#define DHT11_TIMEOUT 400		/* 100 us time out constant */

unsigned char dht11_status;
unsigned char TmpHumReadErrCnt;

#define Delay_us(us)  GCC_DELAY(us*2)

inline void DHT11_PinPullLow()
{
    _pc3=0;
    _pcc3=0;
}
inline void DHT11_PinRelease()
{
    _pcc3=1;
}
inline unsigned char DHT11_PinValue()
{
    return _pc3;
}
unsigned DHT11_ReadByte(void)
{
	unsigned char i, count, value = 0;
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
		Delay_us(40);	/* Delay 30 us */
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
	unsigned char i, count, check_value=0;
	unsigned char value_array[5];
	
	/* Pull data low more than 18 ms */
	DHT11_PinPullLow();
	Delay_us(20000);
	
	/* Release data line, wait 50us and check ACK (DHT11 pull low) */
	_emi=0;
	DHT11_PinRelease();
	Delay_us(50);
	if (DHT11_PinValue() == Bit_SET)
	{
		/* no ACK, terminate read */
		TmpHumReadErrCnt++;
		_emi=1;
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
			TmpHumReadErrCnt++;
			_emi=1;
			return;
		}
		/* Wait until DHT11 pull data line low for first bit*/
		count = 0;		
		while ((DHT11_PinValue() == Bit_SET) && (count++  < DHT11_TIMEOUT));
		if (count >= DHT11_TIMEOUT)
		{
			/* pin is low over time */
			TmpHumReadErrCnt++;
			_emi=1;
			return;
		}
		
		/* Read temperature and humidity */
		for (i=0; i<5; i++)
		{
			value_array[i] = DHT11_ReadByte();
			if (dht11_status == ERROR)
			{
				TmpHumReadErrCnt++;
				_emi=1;
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
			*pHumidity 	= value_array[0];
			TmpHumReadErrCnt=0;
			_emi=1;
			return;
		}
		else
		{	/* checksum error */
			TmpHumReadErrCnt++;
			_emi=1;
			return;
		}
	}
	_emi=1;
}
