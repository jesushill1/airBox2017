;;--------------------------------------------------------------;;
;;    This file is part of the Holtek C Compiler V3 package     ;;
;;    For the initialization of static linkage variables        ;;
;;    Copyright (C) 2013 Holtek Semiconductor Inc.              ;;
;;    Version: 1.05 (Above IDE3000 V7.92)                       ;;
;;    Date:    2016/08/24                                        ;;
;;--------------------------------------------------------------;;

acc equ [05h]
tblp equ [07h]
tblh equ [08h]
;;tbhp equ [09h] 
mp0 equ [01h]
r1 equ [02h]
mp1l equ [03h]
mp1h equ [04h]
z equ [0ah].2
c equ [0ah].0

ifndef tbhp
tbhp equ [09h]
endif
 
extern startup_value_1:near

@start .section 'code'
begin_startup_value:
  mov a,low (offset startup_value_1) 
  mov tblp,a
  mov a,high (offset startup_value_1) 
  mov tbhp,a
next_table:
  ;CLR WDT
  inc tblp
  sz z
  inc tbhp
ifdef USE_TABRD
  tabrd mp0
else
  tabrdc mp0
endif
  sz mp0
  jmp read_data
  jmp end_startup_value
read_data:
  inc tblp
  sz z
  inc tbhp
ifdef USE_TABRD
  tabrd mp1l
else
  tabrdc mp1l
endif
  mov a,tblh
  mov mp1h,a
next_data:
  inc tblp
  sz z
  inc tbhp
ifdef USE_TABRD
  tabrd acc
else
  tabrdc acc
endif
  mov r1,a
  sdz mp0
  jmp $+2
  jmp next_table
  inc mp1l
  mov a,tblh
  mov r1,a
  inc mp1l
  sdz mp0
  jmp next_data
  jmp next_table

end_startup_value:

   
@ROMDATA_BASE .SECTION com_l 'CODE'  
startup_value:
;;linker range the initial value table here