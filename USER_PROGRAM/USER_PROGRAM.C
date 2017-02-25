#include    "USER_PROGRAM.H"  
#include    "bittype.h"
#define     TOUCH_IOPU   _pcpu4
#define     TOUCH_IOC    _pcc4
#define     TOUCH_IO     _pc4                            //TOUCH按键状态输出脚
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
volatile unsigned char  dis_data[4];
volatile unsigned char  data_buffer;

volatile flag_type gv8u_flag1;
#define  key_press_flag   gv8u_flag1.bits.bit0
#define  rx_first_flag    gv8u_flag1.bits.bit1
//#define send_data_flag   gv8u_flag1.bits.bit2             
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
	switch(com_status)	
	{
		case 0:
		{
			data_buffer=dis_data[com_status];	
			COM0=1;	
			break;		
		}
		case 1:
		{
			data_buffer=dis_data[com_status];
			COM1=1;	
			break;		
		}
		case 2:
		{
			data_buffer=dis_data[com_status];
			COM2=1;			
			break;		
		}
		case 3:
		{
			data_buffer=dis_data[com_status];
			COM3=1;
			_frame=~_frame;
			break;		
		}
	}
	if(data_buffer&0x01)
	{
		SEG0=1;
	}
	if(data_buffer&0x02)
	{
		SEG1=1;
	}
	if(data_buffer&0x04)
	{
		SEG2=1;
	}
	if(data_buffer&0x08)
	{
		SEG3=1;
	}
	if(data_buffer&0x10)
	{
		SEG4=1;
	}
	if(data_buffer&0x20)
	{
		SEG5=1;
	}
	com_status++;
	if(com_status>3)
	{
		com_status=0;
	}
	
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
	_pcc=0xff;	
	
	_pdpu=0;
	_pd=0;
	_pdc=0xc0;	
  	
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
	_slcdc0=0b01111111;                   //工作电流100uA,SCOM0~SCOM3使能
	_slcdc1=0b00111111;                   //SSEG0~SSEG5使能
	_slcdc2=0b00000000;
	_slcdc3=0b00000000;
	dis_data[0]=0x00;
		dis_data[1]=0x00;
			dis_data[2]=0x00;
				dis_data[3]=0x00;
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
	if(SCAN_CYCLEF)
	{
   		GET_KEY_BITMAP();
   		if(key_press_flag==0)
   		{
			if(DATA_BUF[0]&&0x80)	
			{
				key_press_flag=1;
				TOUCH_IO=0; 
				TOUCH_IOC=0;                   //有按键，输出0  
			}	
   		}
		else 
		{
			if(DATA_BUF[0]==0)	
			{
				key_press_flag=0; 
				TOUCH_IOPU=1;
				TOUCH_IOC=1;                   //无按键，输入上拉
			}
		}
    }
}