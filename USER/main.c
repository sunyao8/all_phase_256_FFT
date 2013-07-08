#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "adc.h"
#include <math.h>
#include "stm32_dsp.h"

//ALIENTEK战舰STM32开发板实验17
//ADC 实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司   
#define NPT 512            /* NPT = No of FFT point*/
#define FFT_NM NPT/2
#define PI2  6.28318530717959
long lBUFIN_V[NPT];         /* Complex input vector */
long lBUFOUT_V[FFT_NM];        /* Complex output vector */
long lBUFIN_I[NPT];         /* Complex input vector */
long lBUFOUT_I[FFT_NM];        /* Complex output vector */

long lBUFMAG_V[NPT + NPT/2];/* Magnitude vector */
long lBUFMAG_I[NPT + NPT/2];/* Magnitude vector */

double angle[7]; 
void allphase(long *V,long *I);
 int main(void)
 {	 
   	float adcv,adci;
	u16 adc_ix=0,adc_imax=0;
	u16 i=0,flag_v=0,flag_i=0;
	u32 max_v=0,max_i=0;
	int32_t lX,lY;
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	//串口初始化为9600
 	LED_Init();			     //LED端口初始化
	LCD_Init();			 	
 	Adc_Init();		  		//ADC初始化

	POINT_COLOR=RED;//设置字体为红色 
	while(1)
	{

   for(i=0;i<600;i++)
	  {
		 adc_ix=Get_Adc_Average(ADC_Channel_1,2);
		 if(adc_ix>adc_imax)
		 adc_imax=adc_ix;
	  }
			//	LCD_ShowxNum(50+1*25,1*15,adc_imax,4,16,0);

//		LCD_ShowxNum(156,130,adc_imax,4,16,0);//显示ADC的值	 基准电压采样1520
if(adc_imax>=1750)
{
        for(i=0;i<NPT;i++)
        	{
		adci=(Get_Adc_Average(ADC_Channel_4,4)-1520)/10;
		adcv=(Get_Adc_Average(ADC_Channel_1,4)-1520)/10;                //10
		adcv=(Get_Adc_Average(ADC_Channel_1,4)-1520)/10;
		adci=(Get_Adc_Average(ADC_Channel_4,4)-1520)/10;
		lBUFIN_V[i]=((short)adcv) << 16;
		lBUFIN_I[i]=((short)adci) << 16;		
		
        	}

}

if(adc_imax<1750)
{
        for(i=0;i<NPT;i++)
        	{
		adci=(Get_Adc_Average(ADC_Channel_4,4)-1520)/7;
		adcv=(Get_Adc_Average(ADC_Channel_1,4)-1520)/7;                //10
		adcv=(Get_Adc_Average(ADC_Channel_1,4)-1520)/7;
		adci=(Get_Adc_Average(ADC_Channel_4,4)-1520)/7;
		lBUFIN_V[i]=((short)adcv) << 16;
		lBUFIN_I[i]=((short)adci) << 16;		
		
        	}

}
		

			 allphase(lBUFIN_V,lBUFIN_I);
/*
       for(i=0;i<FFT_NM;i++)
        	{
		lBUFIN_V[i]=lBUFIN_V[i]<< 16;
		lBUFIN_I[i]=lBUFIN_I[i] << 16;
        	}


*/


		
          cr4_fft_256_stm32(lBUFOUT_V, lBUFIN_V, FFT_NM);
	   cr4_fft_256_stm32(lBUFOUT_I, lBUFIN_I, FFT_NM);

	
               for(i=0;i<FFT_NM/2;i++)
              {
                 lX  = (lBUFOUT_V[i] << 16) >> 16;
                 lY  = (lBUFOUT_V[i] >> 16);
                  {
                   float X    = FFT_NM* ((float)lX) /32768;
                   float Y    = FFT_NM* ((float)lY) /32768;
                    float Mag = sqrt(X*X + Y*Y)/FFT_NM;
                   lBUFMAG_V[i]    = (u32)(Mag * 65536);
                   }
                 lX  = (lBUFOUT_I[i] << 16) >> 16;
                 lY  = (lBUFOUT_I[i] >> 16);
                  {
                   float X    = FFT_NM* ((float)lX) /32768;
                   float Y    = FFT_NM* ((float)lY) /32768;
                    float Mag = sqrt(X*X + Y*Y)/FFT_NM;
                   lBUFMAG_I[i]    = (u32)(Mag * 65536);
                   }

					
               }


         for(i=1;i<FFT_NM/2;i++)
         	{
               if(lBUFMAG_V[i]>max_v){max_v=lBUFMAG_V[i];flag_v=i;}  

		    if(lBUFMAG_I[i]>max_i){max_i=lBUFMAG_I[i];flag_i=i;}  

		     }



		 
		 lX  = (lBUFOUT_V[flag_v] << 16) >> 16;
                 lY  = (lBUFOUT_V[flag_v] >> 16);
				angle[0]=atan2(lY,lX);

				 lX  = (lBUFOUT_I[flag_i] << 16) >> 16;
                 lY  = (lBUFOUT_I[flag_i] >> 16);
				angle[1]=atan2(lY,lX);
                              angle[5]=((angle[1]-angle[0])*360)/PI2-90;
				angle[2]=sin((angle[1]-angle[0]))*1000;
				if(angle[5]>=0.0&&angle[5]<=180)
					{LCD_ShowxNum(50+1*25,1*15,angle[5],5,16,0);//0~180容性
	                 LCD_ShowString(60,50,200,16,16,"C");	
				        }
				
				else if((angle[5]>-90&&angle[5]<0))
					{LCD_ShowString(60,50,200,16,16,"L");
                           LCD_ShowxNum(50+1*25,2*15,-angle[5],5,16,0);
				}	


                            // if(angle[2]<0)
                             	
				LCD_ShowxNum(5+1*25,4*15,angle[2],4,16,0);
			adc_imax=0;
              delay_ms(20);
	}
 }


void allphase(long *V,long *I)
{
int i=0;
for(i=0;i<=NPT/2-1;i++)
{
V[i]=(i+1)*V[i];
I[i]=(i+1)*I[i];
}
for(i=NPT/2;i<NPT-1;i++)
{
V[i]=(NPT-(i+1))*V[i];
I[i]=(NPT-(i+1))*I[i];

}

for(i=0;i<NPT/2-1;i++)
{
V[i+NPT/2]=V[i]+V[i+NPT/2];
I[i+NPT/2]=I[i]+I[i+NPT/2];

}

for(i=0;i<=NPT/2-1;i++)
{
V[i]=V[NPT/2-1+i];
I[i]=I[NPT/2-1+i];

}
}
