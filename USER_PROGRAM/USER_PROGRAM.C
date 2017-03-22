#include    "USER_PROGRAM.H"
#include    "bittype.h"
#include    "p0TmpHumSensor.h"

//==================================================
#define     TOUCH_IOPU  _pcpu4
#define     TOUCH_IOC   _pcc4
#define     TOUCH_IO    _pc4        //TOUCH按键状态输出脚
#define		LIGHT       _pc2
#define	    FANFBK      _pc6        //FAN feedback
//==================================================
#define     COM0        _pa1
#define     COM1        _pa4
#define     COM2        _pa0
#define     COM3        _pa2
#define     SEG0        _pb0
#define     SEG1        _pb1
#define     SEG2        _pb2
#define     SEG3        _pb3
#define     SEG4        _pb4
#define     SEG5        _pb5
//==================================================
#define 	RX_BUF_LEN 	(11)
#define 	TX_BUF_LEN 	(5)
//==================================================
static volatile unsigned char  	com_status	__attribute__((at(0x3a2)));
static volatile flag_type		gv8u_flag1	__attribute__((at(0x3a3)));
//==================================================
#define  KeyPressed   		gv8u_flag1.bits.bit0
#define  UartRxFrameNull    gv8u_flag1.bits.bit1
#define  KeyLongPressed   	gv8u_flag1.bits.bit2
#define  KeyShortPressed   	gv8u_flag1.bits.bit3
#define  KeyLongPressHold   gv8u_flag1.bits.bit4
#define  FanFbkZ     		gv8u_flag1.bits.bit5
//==================================================
#define  LONG_PRESS_TIME  	3000
#define  SHORT_PRESS_TIME 	200
//==================================================
static volatile unsigned int 	KeyPressTmr 	__attribute__((at(0x380)));
static volatile unsigned char 	DataType 		__attribute__((at(0x382)));
static volatile unsigned char 	LcdSegData[3] 	__attribute__((at(0x383)));
static volatile int 			LcdData 		__attribute__((at(0x386)));
static volatile int 			Temperature 	__attribute__((at(0x388)));
static volatile unsigned int 	DustWeight 		__attribute__((at(0x38a)));
static volatile unsigned int 	Humidity 		__attribute__((at(0x39c)));
//==================================================
// Fan related
static volatile unsigned int    FanSpd          __attribute__((at(0x3B0)));
static volatile unsigned char   FanSpdCnt		__attribute__((at(0x3B2)));
static volatile unsigned int    FanSpdCntTmr	__attribute__((at(0x3B3)));
//==================================================
// RS232 related
static volatile unsigned char	UartRxWaitTmr	__attribute__((at(0x3a5)));
static volatile unsigned char	UartRxFrameCrc	__attribute__((at(0x3a6)));
static volatile unsigned char	UartRxBuf[RX_BUF_LEN]	__attribute__((at(0x3a7)));
static volatile unsigned char	UartTxBuf[TX_BUF_LEN]	__attribute__((at(0x3b2)));
static volatile unsigned char	UartRxBufCnt			__attribute__((at(0x3b7)));
//==================================================
#define SegA 0x80	/* COM3 + SEG1 */
#define SegB 0x40	/* COM2 + SEG1 */
#define SegC 0x20	/* COM1 + SEG1 */
#define SegD 0x10	/* COM0 + SEG1 */
#define SegE 0x01	/* COM0 + SEG0 */
#define SegF 0x04	/* COM2 + SEG0 */
#define SegG 0x02	/* COM1 + SEG0 */
#define SegS 0x08	/* COM3 + SEG0 */
//==================================================
const unsigned char DigSegTable[11] =
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
//==================================================
#define Delay_us(us)  GCC_DELAY(us*2)

void LcdDataRefresh(void);
void LcdRefresh(void);
void FanSpeedRead();
void FanSpeedSet();

inline void DHT11_PinPullLow()
{
    _pc3=0;
    _pcc3=0;
}

inline void DHT11_PinRelease()
{
    _pcc3=1;
}

//==============================================
//**********************************************
//==============================================
//=====CTM0定时器中断===============
static volatile unsigned int 	flag_1ms		=0;
static volatile unsigned char 	flag_readSensor	=0;
static volatile unsigned char 	readingTempHum	=0;
DEFINE_ISR (Interrupt_CTM0A, 0x14)  //1ms
{
#if 1
	if (!readingTempHum) // if not reading DHT11
	{
		flag_1ms++;
	
		if ( (flag_1ms%3==0) ) 
			LcdRefresh();	
		
		if( (flag_readSensor==0) && (flag_1ms==1000) )
		{
			flag_readSensor=1;
			flag_1ms=0;
		}
	}
#endif


#if 0
    // Count the rise edge
    if (FANFBK == 1 && FanFbkZ == 0)
    {
        FanSpdCnt++;
    }
    FanFbkZ = FANFBK;
    // Periodically store the count as fan speed per 1s
    FanSpdCntTmr++;
    if (FanSpdCntTmr >= 1000)
    {
        FanSpd = FanSpdCnt;
        FanSpdCnt = 0;
        FanSpdCntTmr = 0;
    }
#endif
}

//=======UART发送/接收中断===============
static volatile unsigned char index=0;
DEFINE_ISR (Interrupt_Uart, 0x2c)
{
	//接受数据
	if ( _rxif && !_oerr && !_ferr && !_nf )
	{
		unsigned char data;
        // store byte
		data=_txr_rxr;
		
		// reset timeout timer
		UartRxWaitTmr = 0;
        // if it is the first byte
		if(UartRxBufCnt==0)
		{
			if(data==0xA5)
			{
				UartRxBuf[0]=data;
				UartRxBufCnt=1;
				UartRxFrameCrc = 0;
			}
		}
        else if(UartRxBufCnt<(RX_BUF_LEN-1))
		{
			UartRxBuf[UartRxBufCnt]=data;
			UartRxFrameCrc+=data;
			UartRxBufCnt++;
		}
        else if(UartRxBufCnt==RX_BUF_LEN-1)
		{
			UartRxBuf[RX_BUF_LEN-1]=data;
			if(data==UartRxFrameCrc)
			{
                UartRxBufCnt = RX_BUF_LEN;
			}
            else // invalid frame, discard
            {
                UartRxBufCnt = 0;
            }
		}
        else // >=RX_BUF_LEN
        {
            // wait app to handle the frame
        }
	}
	
	
#if 1
	//发送数据
	if ( flag_readSensor && _txif && !_oerr && !_ferr&& !_nf )
	{
		_txr_rxr=UartTxBuf[index++];
		if (index>=5)
		{
			index=0;
			flag_readSensor=0;
		}
	}
#endif
	
	_acc=_usr;
}

void USER_PROGRAM_INITIAL()
{
	_papu=0;
	_pa=0;
	_pac=0b11101000;
	_papu3 = 1;	// RX pin pull-high
	
	_pbpu=0;
	_pb=0;
	_pbc=0xff;
	
	_pcpu=0;
	_pc=0;
	_pcc=0b11111011;    // pc0: NC
                        // pc1: touch input
                        // pc2: light output
                        // pc3: DHT
                        // pc4: NC
                        // pc5: RXD2
                        // pc6: FAN feedback
                        // pc7: TXD2
    _pcpu6 = 1;
	
	//=====CTM0初始化============
	_ctm0c0=0x20;   //fsys/16
	_ctm0c1=0xc1;
	_ctm0al=0xe8;
	_ctm0ah=0x03;   //1000*2us=2ms
	_ct0on=1;
	_ctma0f=0;
	_ctma0e=1;
	
	//=====UART初始化============
	_brg=25;			//9600波特率
	_ucr1=0b10000000;   //UATREN
	//_ucr2=0b11000100;	//TXEN RXEN RIE
	_ucr2=0b11000111;	//TXEN RXEN RIE TIIE TEIE
	_uartf=0;
	_uarte=1;			//UART中断控制位
	
	//=======SCOM_SSEG初始化=====
	_slcdc0=0b00011111;                   //工作电流100uA,SCOM0~SCOM3使能
	_slcdc1=0b00111111;                   //SSEG0~SSEG5使能
	_slcdc2=0b00000000;
	_slcdc3=0b00000000;
    LcdSegData[0]=0x01;
    LcdSegData[1]=0x00;
    LcdSegData[2]=0x00;
    
    //
    DataType = 0; 	//startup,show PM2.5
    KeyPressTmr = 0;
    Temperature = -88;
    Humidity = 99;
    DustWeight = 66;
    
    //
    UartRxBufCnt = 0;
    
    UartTxBuf[0]=0x01;
	UartTxBuf[1]=0x02;
	UartTxBuf[2]=0x03;
	UartTxBuf[3]=0x04;
	UartTxBuf[4]=0x05;
	
	LIGHT=1;
}

void UATR_SEND_DATA()	//UART数据发送
{
	unsigned char i;
	for(i=0;i<5;i++)
	{
		while(!_txif)
		{
			GCC_CLRWDT();
			GCC_CLRWDT1();
			GCC_CLRWDT2();
		}
		_acc=_usr;
		_txr_rxr=UartTxBuf[i];
	}
}

void USER_PROGRAM()
{
	static unsigned int cnt=0;
	static unsigned int cnt_5s=0;
	
	if(SCAN_CYCLEF) //10ms
	{
		cnt++;
		if (cnt>=100) 
		{
			cnt=0;
			if(DataType!=0)
				cnt_5s++;
		}
		
   		GET_KEY_BITMAP();
   		KeyPressed = DATA_BUF[1]&0b00000010 ? 1 : 0;
   		if(!KeyPressed)
   		{
   			if(KeyPressTmr>=LONG_PRESS_TIME)
   			{
   			}
			else if(KeyPressTmr>=SHORT_PRESS_TIME)
   			{
   				KeyShortPressed = 1;
   			}
   			KeyPressTmr = 0;
   			KeyLongPressHold = 0;
   		}
   		else
   		{
   			KeyPressTmr+=30;
   			if(KeyPressTmr>LONG_PRESS_TIME && !KeyLongPressHold)
   			{
   				KeyPressTmr = LONG_PRESS_TIME;
   				KeyLongPressHold = 1;
   				KeyLongPressed = 1;
   			}
   		}
   		// Long key press handler
   		if(KeyLongPressed)
   		{
   			LIGHT = !LIGHT;
   			KeyLongPressed = 0;
   		}
   		// Short key press handler
   		if(KeyShortPressed)
   		{
			#if 1
   			DHT11_PinPullLow();
   			Delay_us(40000);
   			readingTempHum = 1;
   			DHT11_PinRelease();
   			TmpHumRead(&Temperature, &Humidity);
   			LcdDataRefresh();
   			readingTempHum = 0;
   			#endif
   			
   			DataType = (DataType + 1) % 3;
   			KeyShortPressed = 0;
   			LcdDataRefresh();
   		}
   		
   		
		// state machine, in every 10ms
		switch(cnt)
		{
		case 0:
			if (cnt_5s==5)
				DHT11_PinPullLow(); /* Pull data low more than 18 ms */
			break;
		case 20:
			if (cnt_5s==5)
			{
				readingTempHum = 1;
				DHT11_PinRelease(); /* Release data line, wait 50us and check ACK (DHT11 pull low) */
				TmpHumRead(&Temperature, &Humidity);
				readingTempHum = 0;
				cnt_5s = 0;
			}
			break;
        case 40:
            if (UartRxBufCnt == RX_BUF_LEN)
            {
                DustWeight = UartRxBuf[4];
                DustWeight <<= 16;
                DustWeight += UartRxBuf[3];
                UartRxBufCnt=0;
                //LIGHT = !LIGHT;
            }
            break;
        case 60:
        	//UartTxBuf[0]=0x01;
        	//UartTxBuf[1]=0x02;
        	//UartTxBuf[2]=0x03;
        	//UartTxBuf[3]=0x04;
        	//UartTxBuf[4]=0x05;
        	//UATR_SEND_DATA();
        	break;
		case 80:
			LcdDataRefresh();
			break;
        case 99:
            UartRxWaitTmr++;
            if (UartRxWaitTmr>5 && UartRxBufCnt != RX_BUF_LEN)
            {
            	
                UartRxBufCnt = 0;
                UartRxWaitTmr = 0;
            }
            break;
		default:
			GCC_CLRWDT();
			GCC_CLRWDT1();
			GCC_CLRWDT2();
			break;
		}
		
    }
	
	//LcdRefresh();

}

void LcdDataRefresh(void)
{
	unsigned char hundred, ten, digit;
	LcdSegData[0] = 0;
	LcdSegData[1] = 0;
	LcdSegData[2] = 0;
	
	switch(DataType)
	{
		case 1:			// Temperature
			LcdSegData[1] = SegS;
			LcdData = Temperature;		/* Temperautre is 2 digits */
			break;
		case 2:			// Humidity
			LcdSegData[2] = SegS;
			LcdData = Humidity;		/* Humidity is 2 digits */
			break;
		case 0: /* PM2.5 */
			LcdSegData[0] = SegS;
			LcdData = DustWeight;
			break;
		default:
			LcdData = 0;
	}
	// render digitals
	{
		if(LcdData>=0)
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
			if ((hundred != 0))
			{
				LcdSegData[0] += DigSegTable[hundred];
			}
			if(hundred != 0 || ten!=0)
			{
				LcdSegData[1] += DigSegTable[ten];
			}
			LcdSegData[2] += DigSegTable[digit];
		}
		else
		{
			LcdSegData[0]+=DigSegTable[10];
			if(LcdData<-99)
			{
				ten=9;digit=9;
			}
			else
			{
				ten=(-LcdData)/10;
				digit=(-LcdData) % 10;
			}
			if(ten!=0) LcdSegData[1] += DigSegTable[ten];
			LcdSegData[2] += DigSegTable[digit];
		}

	}
}
void LcdRefresh(void)
{
	// LCD refresh
	COM3=0;
	COM2=0;
	COM1=0;
	COM0=0;
	SEG0=0;
	SEG1=0;
	SEG2=0;
	SEG3=0;
	SEG4=0;
	SEG5=0;
	SEG0 = (LcdSegData[0] & (0b00001000 >>com_status))? 1 : 0;
	SEG1 = (LcdSegData[0] & (0b10000000 >>com_status))? 1 : 0;
	SEG2 = (LcdSegData[1] & (0b00001000 >>com_status))? 1 : 0;
	SEG3 = (LcdSegData[1] & (0b10000000 >>com_status))? 1 : 0;
	SEG4 = (LcdSegData[2] & (0b00001000 >>com_status))? 1 : 0;
	SEG5 = (LcdSegData[2] & (0b10000000 >>com_status))? 1 : 0;
	switch(com_status)	
	{
		case 0:
		{
			COM0=1;
			break;
		}
		case 1:
		{
			COM1=1;
			break;
		}
		case 2:
		{
			COM2=1;
			break;
		}
		case 3:
		{
			COM3=1;
			_frame=~_frame;
			break;
		}
	}
	com_status++;
	if(com_status>3)
	{
		com_status=0;
	}
}
