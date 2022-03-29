#ifndef __LOZ_LED8888__
#define __LOZ_LED8888__

#include "asm/cpu.h"


#define   	TCFG_LED8888_ENABLE  				1
#define 	TCFG_LED8888_TEST_ENABLE			0
#define     TCFG_LED8888_ONE_TO_NINE_ENABLE		0


#define   STATE_FREE_MODE         0
#define   STATE_TIMER_COUTDOWN    1
#define   STATE_NUM_COUTDOWN      2
#define   BT_LINK_STATE           3
#define   BATT_REPORT             4



void loz_printf(void);
void led_8888_init(void);
void DISPLAY_NUM_DATA_UPDATE(u32 num, u8 time_num_mode);

void DISPLAY_STATE_DATA_UPDATE(u8 state, u8 var);

void printf_number(void);


/***********test function*****************/
void light_open(void);


#endif
