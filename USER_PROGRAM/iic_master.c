#include "BS82C16A-3.H"
//===================================================================//
#define IO_DIR_SDA 	_pcc6
#define IO_DAT_SDA	_pc6
#define IO_DIR_SCL	_pcc7
#define IO_DAT_SCL	_pc7

#define IIC_ADDR_MASTERWR	0x54     
#define IIC_ADDR_MASTERRD	0x55
//===================================================================//
 volatile unsigned char ac_ad_cnt ;
static volatile unsigned char gv8u_iicmaster_txdata[10] __attribute__ ((at(0x1b0)));
static volatile unsigned char gv8u_iicmaster_rxdata[10] __attribute__ ((at(0x1c0)));
//===================================================================//
void IIC_DELAY(void)
{
	volatile unsigned char i;
	for(i=0;i<7;i++)
	{
		GCC_CLRWDT();
	}
}
//===================================================================//

void fun_iicmaster_start(void)
{
	
	IO_DIR_SCL = 1;
	if(IO_DAT_SCL == 0)
	{
		
		GCC_NOP();
		GCC_NOP();
	}
	else
	{
		GCC_NOP();
		if(IO_DAT_SCL == 1)
		{
			IO_DIR_SDA = 1;
			IIC_DELAY();
			IO_DIR_SCL = 1;
			IO_DAT_SDA = 0;
			IO_DIR_SDA = 0;
			IIC_DELAY();
		}
	}
}
//===================================================================//
void fun_iicmaster_stop(void)
{
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;	
	
	IO_DAT_SDA = 0;
	IO_DIR_SDA = 0;	
	
	IIC_DELAY();
	
	IO_DIR_SCL = 1;
	
	IIC_DELAY();
	IO_DIR_SDA = 1;	
	
}
//===================================================================//
unsigned char fun_iicmaster_bytewrite(unsigned char lv8u_data)
{
	volatile unsigned char i,j,k,lv8u_writeok;
	lv8u_writeok = 1;

	k=0;
	for(i=0;i<8;i++)
	{
		IO_DAT_SCL = 0;
		IO_DIR_SCL = 0;	
		if(lv8u_data&0x80)
		{
			IO_DIR_SDA = 1;
		}
		else
		{
			IO_DAT_SDA = 0;
			IO_DIR_SDA = 0;		
		}
		IIC_DELAY();
		IO_DIR_SCL = 1;	
		
		if(i == 0)				//iic bus idle check
		{
			while(1)
			{
				if(IO_DAT_SCL == 0)
				{
					GCC_NOP();
					for(j=0;j<100;j++);
					k++;
					if(k>20)
					{
						lv8u_writeok = 0;
						break;
					}
					
				}
				else
				{
					break;
				}
			}
		}
		if(lv8u_writeok == 0)
		{
			break;
		}
		
		IIC_DELAY();
		lv8u_data = lv8u_data<<1;
	}
	
	return lv8u_writeok;
}
//===================================================================//
unsigned char fun_iicmaster_rxak(void)
{
	unsigned char i,j;
	j = 0;	
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;	
	
	IO_DIR_SDA = 1;
	IIC_DELAY();
	IO_DIR_SCL = 1;	
	GCC_NOP();
	GCC_NOP();
	for(i=0;i<4;i++)
	{
		if(IO_DAT_SDA == 0)
		{
			j++;
		}
	}
	
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;
	
	if(j>2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//===================================================================//
unsigned char fun_iicmaster_byteread(void)
{
	unsigned char i,lv8u_data;
	lv8u_data = 0;
	for(i=0;i<8;i++)
	{	
		lv8u_data = lv8u_data<<1;
		
		IO_DAT_SCL = 0;
		IO_DIR_SCL = 0;
		IO_DIR_SDA = 1;			
		IIC_DELAY();
		IO_DIR_SCL = 1;	
		IIC_DELAY();
		if(IO_DAT_SDA == 1)
		{
			lv8u_data = lv8u_data|0x01;
		}			
	}
	return lv8u_data;
}
//===================================================================//
void fun_iicmaster_txak(void)
{
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;		
	IIC_DELAY();
	IO_DAT_SDA = 0;
	IO_DIR_SDA = 0;
	
	IO_DIR_SCL = 1;	
	IIC_DELAY();
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;		
}
//===================================================================//
void fun_iicmaster_txnoak(void)
{
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;		
	IIC_DELAY();
	IO_DIR_SDA = 1;
	
	IO_DIR_SCL = 1;	
	IIC_DELAY();
	IO_DAT_SCL = 0;
	IO_DIR_SCL = 0;		
}
//===================================================================//
void fun_iicmaster_sendbytes(unsigned char lv8u_senlen)
{
	unsigned char i;
//	pbuf = gv8u_iicmaster_txdata;
		gv8u_iicmaster_txdata[0] =0xb0;//gv8u_iicmaster_rxdata[0];
		gv8u_iicmaster_txdata[1] =55;//gv8u_iicmaster_rxdata[1];
		gv8u_iicmaster_txdata[2] = 55;//gv8u_iicmaster_rxdata[2];
		gv8u_iicmaster_txdata[3] = 55;//gv8u_iicmaster_rxdata[3];
		gv8u_iicmaster_txdata[4] =55;//gv8u_iicmaster_rxdata[4];
		gv8u_iicmaster_txdata[5] = 55;//gv8u_iicmaster_rxdata[5];
		gv8u_iicmaster_txdata[6] = 55;//gv8u_iicmaster_rxdata[6];
		gv8u_iicmaster_txdata[7] =55;//gv8u_iicmaster_rxdata[7];
		gv8u_iicmaster_txdata[8] = 55;//gv8u_iicmaster_rxdata[8];
		gv8u_iicmaster_txdata[9] = 55;//gv8u_iicmaster_rxdata[9];
	fun_iicmaster_start();
	fun_iicmaster_bytewrite(IIC_ADDR_MASTERWR);
	if(fun_iicmaster_rxak())
	{

		for(i=0;i<lv8u_senlen;i++)
		{
			
			fun_iicmaster_bytewrite(gv8u_iicmaster_txdata[i]);
			if(fun_iicmaster_rxak())
			{
//				pbuf++
				IIC_DELAY();
				GCC_CLRWDT();
			}
			else
			{
				GCC_CLRWDT();
				break;
			}
		}
	}

	fun_iicmaster_stop();
	
}

//=======================================================================//
//===================================================================//
void fun_iicmaster_recbytes(unsigned char lv8u_reclen)
{
//	unsigned char *pbuf;
	unsigned char i;
//	pbuf = &gv8u_iicmaster_rxdata[0];
	fun_iicmaster_start();
	fun_iicmaster_bytewrite(IIC_ADDR_MASTERRD);
	if(fun_iicmaster_rxak())
	{
		for(i=0;i<lv8u_reclen-1;i++)
		{
		//	*pbuf = fun_iicmaster_byteread();
		    gv8u_iicmaster_rxdata[i] = fun_iicmaster_byteread();
			fun_iicmaster_txak();	
			IIC_DELAY();
		//	pbuf++;					
		}
			gv8u_iicmaster_rxdata[i] = fun_iicmaster_byteread();
			fun_iicmaster_txnoak();
	}
	fun_iicmaster_stop();	
/*	return gv8u_iicmaster_rxdata;*/
}