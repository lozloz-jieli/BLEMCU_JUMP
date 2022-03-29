//#include <stdio.h>
#include "7PIN_led8888.h"
#include "system/includes.h"

#include "asm/includes.h"
#include "app_config.h"
#include "device/key_driver.h"
#include "asm/power/p33.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "btstack/bluetooth.h"
#include "app_power_manage.h"
#include "gpio.h"
#define LOG_TAG_CONST       SPP_AND_LE
#define LOG_TAG             "[SPP_AND_LE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define test_led8888 0
u16 led8888_scan_t = 0;



/************这几个io口验证完毕***********/
//u8 led_port[7]={IO_PORTB_01,IO_PORTB_08,IO_PORTB_00,IO_PORTB_06,IO_PORTB_05,IO_PORTB_04,IO_PORTB_03};
//u8 led_port[7]={IO_PORTB_04,IO_PORTB_05,IO_PORTB_06,IO_PORTB_07,IO_PORTA_00,IO_PORTA_01,IO_PORTA_02};  //first version,
//enhance drive version,
u8 led_port[7]={IO_PORTB_04,IO_PORTA_07,IO_PORTB_06,IO_PORTA_08,IO_PORTA_09,IO_PORTA_01,IO_PORTA_02};

/*********关灯**********/
void LED8888_close_init(void)
{
    for(u8 i=0;i<7;i++)
    {
        u8 port=led_port[i];
        gpio_set_pull_down(port, 0);
        gpio_set_pull_up(port, 0);
        gpio_set_direction(port, 1);
		gpio_set_hd(port, 0);
    }

}

#define PIN1_H  gpio_direction_output(led_port[0], 1)
#define PIN1_L  gpio_direction_output(led_port[0], 0)
#define  PIN1_HD gpio_set_hd(led_port[0], 1)
 

#define PIN2_H  gpio_direction_output(led_port[1], 1)
#define PIN2_L  gpio_direction_output(led_port[1], 0)
#define  PIN2_HD gpio_set_hd(led_port[1], 1)


#define PIN3_H  gpio_direction_output(led_port[2], 1)
#define PIN3_L  gpio_direction_output(led_port[2], 0)
#define  PIN3_HD gpio_set_hd(led_port[2], 1)


#define PIN4_H  gpio_direction_output(led_port[3], 1)
#define PIN4_L  gpio_direction_output(led_port[3], 0)
#define  PIN4_HD gpio_set_hd(led_port[3], 1)


#define PIN5_H  gpio_direction_output(led_port[4], 1)
#define PIN5_L  gpio_direction_output(led_port[4], 0)
#define  PIN5_HD gpio_set_hd(led_port[4], 1)


#define PIN6_H  gpio_direction_output(led_port[5], 1)
#define PIN6_L  gpio_direction_output(led_port[5], 0)
#define  PIN6_HD gpio_set_hd(led_port[5], 1)


#define PIN7_H  gpio_direction_output(led_port[6], 1)
#define PIN7_L  gpio_direction_output(led_port[6], 0)
#define  PIN7_HD gpio_set_hd(led_port[6], 1)


/*
- k1	-计时 k5
- k2	-计数 k6
- k3	-蓝牙 k7
*/
#define LED_A   BIT(0)
#define LED_B	BIT(1)
#define LED_C	BIT(2)
#define LED_D	BIT(3)
#define LED_E	BIT(4)
#define LED_F	BIT(5)
#define LED_G	BIT(6)

#define LED_K1   	BIT(0)
#define LED_K2		BIT(1)
#define LED_K3		BIT(2)
#define LED_K4   	BIT(3)
#define LED_K5		BIT(4)
#define LED_K6		BIT(5)
#define LED_K7		BIT(6)
#define LED_K8		BIT(7)

//#define LED_D1	    BIT(7)


#if 0    //搁置先  app那边传信息回来才能做
#define COUNT	BIT(0)
#define TIMER	BIT(1)
#define BTLED	BIT(2)
#define MAOHA	BIT(3)     //冒号显示
#endif


static const  u8 LED_NUMBER[10] = {
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F), 		 //'0'   
    (u8)(LED_B | LED_C), 										 //'1'
    (u8)(LED_A | LED_B | LED_D | LED_E | LED_G), 				 //'2'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_G),  				 //'3'
    (u8)(LED_B | LED_C | LED_F | LED_G),						 //'4'
    (u8)(LED_A | LED_C | LED_D | LED_F | LED_G), 				 //'5'
    (u8)(LED_A | LED_C | LED_D | LED_E | LED_F | LED_G), 		 //'6'
    (u8)(LED_A | LED_B | LED_C), 								 //'7'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G), //'8'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_F | LED_G), 		 //'9'
};

static const u8 POWR_PIC[3] = {
	(u8) 0,                  //   LOW
	(u8)(LED_K3),
	(u8)((LED_K3) | (LED_K2)),
	(u8)((LED_K3) | (LED_K2) | (LED_K1)),
};

 static  u8 DISPLAY_DATA_BUFFER[5];

#if 0  	//搁置先  app那边传信息回来才能做
static const u8 CHAR_PIC[3] = {
	(u8) (K3),
	(u8)((K3) | (K2)),
	(u8)((K3) | (K2) | (K1)),
};
#endif


void printf_number(void)
{
	u8 i;
	for(i = 0; i < 8; i++){
		r_printf("the led %d  number %d",i,LED_NUMBER[i]);
	}
}

#if test_led8888
u8 number_light[5];

void buffer_recept(void)
{
	number_light[0] = LED_NUMBER[9];    //显示8
	number_light[1] = LED_NUMBER[9];	//显示8
	number_light[2] = LED_NUMBER[9];	//显示8
	number_light[3] = LED_NUMBER[9];	//显示4
	number_light[4] = 0xff;//LED_NUMBER[9];	//显示4
}
#endif

/************管脚单独点亮*************/
void light_open(void)
{
	static u8 flag = 0;
	if(flag == 0){
		PIN2_H;
		PIN4_L;
		PIN3_L;
		PIN1_L;
		PIN7_L;
		PIN5_L;
		PIN6_L;
	}else if(flag == 1){
		PIN1_H;
		PIN4_L;
		PIN3_L;
		PIN2_L;
		PIN7_L;
		PIN5_L;
		PIN6_L;
	}else if(flag == 2){
		PIN1_L;
		PIN2_L;
		PIN3_H;
		PIN4_L;
		PIN5_L;
		PIN6_L;
		PIN7_L;
	}else if(flag == 3){
		PIN1_L;
		PIN2_L;
		PIN3_L;
		PIN4_H;
		PIN5_L;
		PIN6_L;
		PIN7_L;
	}else if(flag == 4){
		PIN1_L;
		PIN2_L;
		PIN3_L;
		PIN4_L;
		PIN5_H;
		PIN6_L;
		PIN7_L;
	}else if(flag == 5){
		PIN1_L;
		PIN2_L;
		PIN3_L;
		PIN4_L;
		PIN5_L;
		PIN6_H;
		PIN7_L;
	}else if(flag == 5){
		PIN1_L;
		PIN2_L;
		PIN3_L;
		PIN4_L;
		PIN5_L;
		PIN6_L;
		PIN7_H;
	}
	flag++;
	if(flag > 5){
		flag = 0;
	}
}

/*****************单独点亮个个数字口test**************************/

void zerotonine_char_display(void)    		//ok
{
	LED8888_close_init();
	static u8 cnt = 0;
		if(cnt == 0){           //1A
			PIN2_H;
			PIN4_L;
		}
		if(cnt == 1){			//1B
			PIN4_H;
			PIN2_L;
		}
		if(cnt == 2){			//1C
			PIN2_H;
			PIN3_L;
		}
		if(cnt == 3){			//1D
			PIN1_H;
			PIN2_L;
		}
		if(cnt == 4){			//1E
			PIN2_H;
			PIN1_L;
		}
		if(cnt == 5){			//1F
			PIN3_H;
			PIN2_L;
		}
		if(cnt == 6){			//1G
			PIN3_H;
			PIN1_L;
		}
		cnt++;
		if(cnt == 7){
			cnt = 0;
		}

}

void zerotonine2_char_display(void)
{
	LED8888_close_init();
	static u8 cnt = 0;
		if(cnt == 0){           //2A
			PIN2_H;
			PIN7_L;
		}
		if(cnt == 1){			//2B
			PIN2_H;
			PIN5_L;
		}
		if(cnt == 2){			//2C
			PIN4_H;
			PIN5_L;
		}
		if(cnt == 3){			//2D
			PIN3_H;
			PIN4_L;
		}
		if(cnt == 4){			//2E
			PIN5_H;
			PIN3_L;
		}
		if(cnt == 5){			//2F
			PIN4_H;
			PIN3_L;
		}
		if(cnt == 6){			//2G
			PIN3_H;
			PIN5_L;
		}
		cnt++;
		if(cnt == 7){
			cnt = 0;
		}

}

void zerotonine3_char_display(void)
{
	LED8888_close_init();
	static u8 cnt = 0;
		if(cnt == 0){           //3A
			PIN6_H;
			PIN2_L;
		}
		if(cnt == 1){			//3B
			PIN6_H;
			PIN3_L;
		}
		if(cnt == 2){			//3C
			PIN5_H;
			PIN6_L;
		}
		if(cnt == 3){			//3D
			PIN6_H;
			PIN5_L;
		}
		if(cnt == 4){			//3E
			PIN6_H;
			PIN4_L;
		}
		if(cnt == 5){			//3F
			PIN3_H;
			PIN6_L;
		}
		if(cnt == 6){			//3G
			PIN4_H;
			PIN6_L;
		}
		cnt++;
		if(cnt == 7){
			cnt = 0;
		}

}

void zerotonine4_char_display(void)
{
	LED8888_close_init();
	static u8 cnt = 0;
		if(cnt == 0){           //4A
			PIN3_H;
			PIN7_L;
		}
		if(cnt == 1){			//4B
			PIN7_H;
			PIN4_L;
		}
		if(cnt == 2){			//4C
			PIN6_H;
			PIN7_L;
		}
		if(cnt == 3){			//4D
			PIN7_H;
			PIN6_L;
		}
		if(cnt == 4){			//4E
			PIN7_H;
			PIN5_L;
		}
		if(cnt == 5){			//4F
			PIN4_H;
			PIN7_L;
		}
		if(cnt == 6){			//4G
			PIN5_H;
			PIN7_L;
		}
		cnt++;
		if(cnt == 7){
			cnt = 0;
		}

}



#define DISPLAY_BALANCE 0
#define DISPLAY_K       1


void led8888_scan()
{
	static u8 cnt = 0;
	u8* buf_dat;
	u8 power_level = 0;
	//r_printf("here are the scan");

#if test_led8888
/*************数字显示********************/
	buffer_recept();
	buf_dat =  number_light;
	//u8 buf_dat[4] = {LED_NUMBER[8],LED_NUMBER[8],LED_NUMBER[8],LED_NUMBER[8]};
/*************电量显示********************/
	//power_level =get_battery_level();
	//buf_dat[4] = POWR_PIC[power_level];
	//*(buf_dat + 4) = 0x55;//POWR_PIC[3];    //直接先给满电
#else
	buf_dat = DISPLAY_DATA_BUFFER;//上的项目填写在这里
	
 
#endif

	LED8888_close_init();
	switch(cnt){                                   //将cnt作为扫描io口
	case 0:
		PIN1_H;
	    PIN1_HD;
	
		if(buf_dat[0] & LED_A){
			PIN2_L;
		}
		if(buf_dat[0] & LED_B){
			PIN3_L;
		}

		if(buf_dat[0] & LED_E){
			PIN4_L;
		}
#if DISPLAY_K
		if(buf_dat[4] & LED_K4){
			PIN5_L;
		}

		if(buf_dat[4] & LED_K6){
			PIN6_L;
		}		
//2个
#endif
       //为了亮度均
	#if DISPLAY_BALANCE 	
	
		PIN7_H;
		if(buf_dat[3] & LED_D){
			PIN6_L;
		}
	#endif	
		break;
	case 1:
		PIN2_H;
		PIN2_HD;
	//	log_info("ble jimmy DISPLAY_DATA_BUFFER:%d\n",buf_dat[4]);
		if(buf_dat[0] & LED_F){
			PIN1_L;
		}
		if(buf_dat[1] & LED_A){
			PIN3_L;
		}
		if(buf_dat[1] & LED_B){
			PIN4_L;
			//  log_info("LED6 -state state\n");
		}
		
		//PIN2_H;
		  if(buf_dat[1] & LED_D){
			PIN6_L;
		}
		if(buf_dat[1] & LED_E){
			PIN5_L;
		}   

		break;
#if DISPLAY_BALANCE
		case 8:
			PIN2_H;
		  if(buf_dat[0] & LED_A){
			PIN4_L;
		}//移到第7个去
		if(buf_dat[0] & LED_C){
			PIN3_L;
		}   
		if(buf_dat[0] & LED_E){
			PIN1_L;
		}
			break;
	#endif 	
	case 2:
		PIN3_H;
		PIN3_HD;
		if(buf_dat[0] & LED_G){
			PIN1_L;
		}
		if(buf_dat[1] & LED_F){
			PIN2_L;
		}
		if(buf_dat[2] & LED_B){
			PIN5_L;    
		}

#if DISPLAY_K
		if(buf_dat[4] & LED_K1){
			PIN6_L;
		}
		if(buf_dat[4] & LED_K7){
			PIN7_L;
		}
		if(buf_dat[4] & LED_K5){
			PIN4_L;    
		}

//三个
#endif
		break;
	#if DISPLAY_BALANCE
		case 9:
			PIN3_H;
		if(buf_dat[0] & LED_F){
			PIN2_L;
		}
		if(buf_dat[0] & LED_G){
			PIN1_L;
		}
		if(buf_dat[1] & LED_D){
			PIN4_L;
		}
			break;
	#endif	
	case 3:
		PIN4_H;
		PIN4_HD;
		if(buf_dat[0] & LED_C){
			PIN1_L;
		}
		if(buf_dat[1] & LED_G){
			PIN2_L;
		}
		if(buf_dat[2] & LED_C){
			PIN5_L;
		}
		if(buf_dat[2] & LED_F){
			PIN3_L;
		}
		if(buf_dat[3] & LED_E){
			PIN6_L;
		}
		break;
	case 4:
		PIN5_H;
		PIN5_HD;
		if(buf_dat[0] & LED_D){
			PIN1_L;
		}
		if(buf_dat[1] & LED_C){
			PIN2_L;
		}
		if(buf_dat[2] & LED_A){
			PIN4_L;
		}
		if(buf_dat[2] & LED_G){
			PIN3_L;
		}
	   if(buf_dat[3] & LED_C){
			PIN6_L;
		}
		if(buf_dat[3] & LED_G){
			PIN7_L;
		}
		break;
#if DISPLAY_BALANCE
		case 7:
		   PIN5_H;
			if(buf_dat[1] & LED_E){
				PIN3_L;
			}
	   if(buf_dat[4] & LED_K3){
			PIN2_L;
		}
		if(buf_dat[4] & LED_K4){
			PIN4_L;
		}
			break;
	#endif 	

	case 5:
		PIN6_H;
		PIN6_HD;
		if(buf_dat[2] & LED_D){
			PIN1_L;
		}
		if(buf_dat[2] & LED_E){
			PIN3_L;
		}
		if(buf_dat[3] & LED_B){
			PIN7_L;
		}
		if(buf_dat[3] & LED_D){
			PIN4_L; 
		}
		if(buf_dat[3] & LED_F){
			PIN5_L;
		}
#if DISPLAY_K
		if(buf_dat[4] & LED_K3){
		  PIN2_L;
		}

  //一个k
#endif
		break;
	case 6:
		PIN7_H;
	    PIN7_HD;
		if(buf_dat[3] & LED_A){
			PIN6_L;
		} // 移到第0个去
//		if(buf_dat[3] & LED_E){
//			PIN5_L;
//		}
//		if(buf_dat[3] & LED_B){
//			PIN4_L;
//		}
//
//		if(buf_dat[4] & LED_K5){
//			PIN3_L;
//		}
//		if(buf_dat[4] & LED_K7){
//			PIN2_L;
//		}

#if DISPLAY_K
		if(buf_dat[4] & LED_K2){
			PIN3_L;
		}
		if(buf_dat[4] & LED_K8){
			PIN2_L;
		}		
//两个k
#endif
		break;
		/// jimmy add 
	
	default:
        break;
	}
/**************做成循环*****************/
	cnt ++;
	if(cnt == 8){
		cnt = 0;
	}

}



extern u16 g_auto_shutdown_timer;

/*******************传参显示数字***************************************/
// 1为时间模式，
void DISPLAY_NUM_DATA_UPDATE(u32 num, u8 time_num_mode)
{
   u16 min;
   u16 sec;
   y_printf("DISPLAY_NUM_DATA_UPDATE -stime_num_mode=%d\n",time_num_mode);
  
  #if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //重置无操作定时计数
   
        sys_timer_modify(g_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    
#endif
 
	if(time_num_mode ==STATE_TIMER_COUTDOWN)   // 
	{
		min =num/60;
		sec =num%60;
			if(min<10)
			{
				DISPLAY_DATA_BUFFER[0] =0; // 灭
			}
			else
			{
				DISPLAY_DATA_BUFFER[0] =LED_NUMBER[(min /10)%10];
			}

			if(min==0)
			{
				DISPLAY_DATA_BUFFER[1] =0; // 灭
			}
			else
			{
				DISPLAY_DATA_BUFFER[1] =LED_NUMBER[min %10];
			}

		DISPLAY_DATA_BUFFER[2] =LED_NUMBER[(sec /10)%10];
		DISPLAY_DATA_BUFFER[3] = LED_NUMBER[num %10];
	}
   
	else if (time_num_mode==0xff)  // close all
	{ 
		DISPLAY_DATA_BUFFER[0] =0;
		DISPLAY_DATA_BUFFER[1] =0;
		DISPLAY_DATA_BUFFER[2] =0;
		DISPLAY_DATA_BUFFER[3] =0;
		DISPLAY_DATA_BUFFER[4] =0;
	}
	else   // display min and secomod
	{
		
		if(num<1000)
		{
			DISPLAY_DATA_BUFFER[0] =0; // 灭
		}
		else
		{
			DISPLAY_DATA_BUFFER[0] =LED_NUMBER[(num /1000)%10];
		}

		if(num<100)
		{
			DISPLAY_DATA_BUFFER[1] =0; // 灭
		}
		else
		{
			DISPLAY_DATA_BUFFER[1] =LED_NUMBER[(num /100)%10];
		}

		if(num<10)
		{
			DISPLAY_DATA_BUFFER[2] =0; // 灭
		}
		else 
		{
			DISPLAY_DATA_BUFFER[2] =LED_NUMBER[(num /10)%10];
		}

		DISPLAY_DATA_BUFFER[3] = LED_NUMBER[num %10];
	}   	

	
   y_printf("DISPLAY_DATA_BUFFER[0] =%d,  DISPLAY_DATA_BUFFER[1] =%d",DISPLAY_DATA_BUFFER[0],DISPLAY_DATA_BUFFER[1]);
	y_printf("DISPLAY_DATA_BUFFER[2] =%d,  DISPLAY_DATA_BUFFER[3] =%d",DISPLAY_DATA_BUFFER[2],DISPLAY_DATA_BUFFER[3]);
 		 
}

void DISPLAY_STATE_DATA_UPDATE(u8 state, u8 var)
{

  g_printf("DISPLAY_STATE_DATA_UPDATE var=%d\n",var);
   if(state ==BATT_REPORT)  //更新电量
   	{
   	DISPLAY_DATA_BUFFER[4] =  (DISPLAY_DATA_BUFFER[4]&0XD3); // 先清，后更新      	k3 k4 k6 LED  0b 1101 0011   
   	if(var==1)
		DISPLAY_DATA_BUFFER[4] |=LED_K4;
	else if(var==2)
		DISPLAY_DATA_BUFFER[4] =DISPLAY_DATA_BUFFER[4]|LED_K4|LED_K3;
	else if(var==3)
		DISPLAY_DATA_BUFFER[4] =DISPLAY_DATA_BUFFER[4]|LED_K4|LED_K3|LED_K6;
   	}
   else if(state ==STATE_TIMER_COUTDOWN)
   	{
   	  DISPLAY_DATA_BUFFER[4] =  (DISPLAY_DATA_BUFFER[4]&0XEC)|LED_K5|LED_K1 ;      //  // K1 k2 K5  1  2 5bit    loz:冒号和计数提示,清掉计数
   	  log_info("DISPLAY_STATE_DATA_UPDATE -STATE_TIMER_COUTDOWN DISPLAY_DATA_BUFFER[4]=%d\n",DISPLAY_DATA_BUFFER[4] );
   	}
	else if(state ==STATE_NUM_COUTDOWN)
   	  {
   	  	   DISPLAY_DATA_BUFFER[4] =  (DISPLAY_DATA_BUFFER[4]&0XEC)|LED_K2;  //   K4 K5  3、4 5bit
   	  	   log_info("DISPLAY_STATE_DATA_UPDATE -STATE_NUM_COUTDOWN DISPLAY_DATA_BUFFER[4]=%d\n",DISPLAY_DATA_BUFFER[4] );
   	  }
	
   else if(state ==STATE_FREE_MODE)
   	{
   	    DISPLAY_DATA_BUFFER[4] &= 0XEC;
   	}
   else if(state ==BT_LINK_STATE)
   	{
   	 if(var)
   	    DISPLAY_DATA_BUFFER[4] |= LED_K7;   // 第七位 //K7
   	 else
	 	DISPLAY_DATA_BUFFER[4] &=  0xbf;   // 第七位 //K7
   	}
   log_info("DISPLAY_STATE_DATA_UPDATE [3]=%d ,[4]==%d \n",DISPLAY_DATA_BUFFER[3],DISPLAY_DATA_BUFFER[4]);
}

void led_8888_init(void)
{
	LED8888_close_init();

#if TCFG_LED8888_ENABLE
	if(led8888_scan_t == 0){
		led8888_scan_t = sys_hi_timer_add(NULL,led8888_scan , 2);
	}
#endif

#if 0//TCFG_LED8888_TEST_ENABLE
	if(led8888_scan_t == 0){
		led8888_scan_t = sys_hi_timer_add(NULL,light_open , 200);
	}

#endif

#if 0//TCFG_LED8888_ONE_TO_NINE_ENABLE
		if(led8888_scan_t == 0){
			led8888_scan_t = sys_hi_timer_add(NULL,zerotonine4_char_display , 200);
		}

#endif

}




void loz_printf(void)
{
    printf("hello world",__func__);
}
