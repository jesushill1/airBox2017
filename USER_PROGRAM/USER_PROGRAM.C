#include    "USER_PROGRAM.H"  
#include    "bittype.h"
#include    "p0TmpHumSensor.h"
#define     TOUCH_IOPU   _pcpu4
#define     TOUCH_IOC    _pcc4
#define     TOUCH_IO     _pc4                            //TOUCH按键状态输出脚
#define     LIGHT        _pc2
//==================================================
#define     COM0         _pa1
#define     COM1         _pa4
#define     COM2         _pa0
#define     COM3         _pa2
#define     SEG0         _pb0
#define     SEG1         _pb1
#define     SEG2         _pb2
#define     SEG3         _pb3
#define     SEG4         _pb4
#define     SEG5         _pb5


volatile unsigned char  rx_databuf[5];
volatile unsigned char  rx_data[5]; 
volatile unsigned char  tx_databuf[5];
volatile unsigned char  tx_data[5]; 
volatile unsigned char  rx_cnt; 
volatile unsigned char  rx_time_cnt; 
volatile unsigned char  com_status;
volatile flag_type gv8u_flag1;



#define  KeyPressed   gv8u_flag1.bits.bit0
#define  rx_first_flag    gv8u_flag1.bits.bit1
#define  KeyLongPressed   gv8u_flag1.bits.bit2
#define  KeyShortPressed   gv8u_flag1.bits.bit3
#define  KeyLongPressHold   gv8u_flag1.bits.bit4

#define  LONG_PRESS_TIME  3000
#define  SHORT_PRESS_TIME 200

static volatile unsigned int 	KeyPressTmr 	__attribute__((at(0x380)));
static volatile unsigned char 	DataType 		__attribute__((at(0x382)));
static volatile unsigned char 	LcdSegData[3] 	__attribute__((at(0x383)));
static volatile int 			LcdData 		__attribute__((at(0x386)));
static volatile int 			Temperature 	__attribute__((at(0x388)));
static volatile unsigned int 	DustWeight 		__attribute__((at(0x38a)));
static volatile unsigned int 	Humidity 		__attribute__((at(0x39c)));


#define SegA 0x80	/* COM3 + SEG1 */
#define SegB 0x40	/* COM2 + SEG1 */
#define SegC 0x20	/* COM1 + SEG1 */
#define SegD 0x10	/* COM0 + SEG1 */
#define SegE 0x01	/* COM0 + SEG0 */
#define SegF 0x04	/* COM2 + SEG0 */
#define SegG 0x02	/* COM1 + SEG0 */
#define SegS 0x08	/* COM3 + SEG0 */

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
void LcdDataRefresh(void);
void LcdRefresh(void);
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
//==============================================
//**********************************************
//==============================================
//=====CTM0定时器中断===============
DEFINE_ISR (Interrupt_CTM0A, 0x14)  //2ms
{
	if(rx_time_cnt<4)
	{
		rx_time_cnt++;
	}
	else
	{
		rx_first_flag=1;            //若长时间未收到数据，则认为此帧数据传输结束
	}
	//LcdRefresh();

}
//=======UART接收中断===============
DEFINE_ISR (Interrupt_Uart, 0x2c)
{
	if(_rxif&&!_oerr&&!_ferr&&!_nf)
	{	
		if(rx_first_flag)
		{
			rx_cnt=0;
			rx_first_flag=0;
			rx_time_cnt=0;
		}
		if(rx_cnt<5)
		{	
			rx_databuf[rx_cnt]=_txr_rxr;
			rx_cnt++;
			if(rx_cnt==5)
			{
				rx_cnt=0;
			}
		}
	}
	_acc=_usr;
}
//==============================================
//**********************************************
//==============================================
void USER_PROGRAM_INITIAL()
{	
	_papu=0;
	_pa=0;
	_pac=0b11101000;
	
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
	
	//=====CTM0初始化============
	_ctm0c0=0x20;   //fsys/16
	_ctm0c1=0xc1;
	_ctm0al=0xe8;
	_ctm0ah=0x03;   //1000*2us=2ms
	_ct0on=1;
	_ctma0f=0;
	_ctma0e=1;
	
	//=====UART初始化============
	_brg=12;                              //9600波特率
	_ucr1=0b10000000;
	_ucr2=0b11000100;	   
	_uartf=0;
	_uarte=1;	
	
	//=======SCOM_SSEG初始化=====
	_slcdc0=0b00011111;                   //工作电流100uA,SCOM0~SCOM3使能
	_slcdc1=0b00111111;                   //SSEG0~SSEG5使能
	_slcdc2=0b00000000;
	_slcdc3=0b00000000;
    LcdSegData[0]=0x01;
    LcdSegData[1]=0x00;
    LcdSegData[2]=0x00;
    
    //
    DataType = 0;
    KeyPressTmr = 0;
    Temperature = -99;
    Humidity = 99;
    DustWeight = 100;
}
//==============================================
//**********************************************
//==============================================
void UATR_SEND_DATA()                    //UART数据发送
{
	unsigned char i;
//	if(send_data_flag)
//	{
//		send_data_flag=0;
		for(i=0;i<5;i++)
		{
			while(!_txif)
			{
				GCC_CLRWDT();
				GCC_CLRWDT1();
				GCC_CLRWDT2();
			}		
			_acc=_usr;
			_txr_rxr=tx_databuf[i];		
		}
/*	}*/
}
//==============================================
//**********************************************
//==============================================
void USER_PROGRAM()
{
	static unsigned char cnt=0;
	static unsigned char st=0;
	if(SCAN_CYCLEF)
	{
		cnt++;
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
   		// Long key press handler
   		if(KeyShortPressed)
   		{
   			DataType = (DataType + 1) % 3;
   			KeyShortPressed = 0;
   			LcdDataRefresh();
   		}
   		LcdRefresh();
    }
    
	switch(cnt)
	{
	case 0:
		DHT11_PinPullLow();
		break;
	case 1:
		DHT11_PinRelease();
		TmpHumRead(&Temperature, &Humidity);
		break;
	case 2:
		LcdDataRefresh();
		break;
	default:
		break;
	}
	if(cnt>=20)
	{
		cnt=0;
	}
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