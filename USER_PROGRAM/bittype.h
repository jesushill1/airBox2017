
typedef struct{
				unsigned char bit0:1;
				unsigned char bit1:1;
				unsigned char bit2:1;
				unsigned char bit3:1;
				unsigned char bit4:1;
				unsigned char bit5:1; 
				unsigned char bit6:1;
				unsigned char bit7:1; 
				}bit_type;
				
typedef union{
			  bit_type 			bits;
			  unsigned char		byte; 
			  }flag_type;
			  
			  

