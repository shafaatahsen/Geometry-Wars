#include <stdint.h>
#include "SysTick.h"
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "ADC.h"

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

//********************SYSTICK HANDLER *****************//
void SysTick_Handler(void){
	PF2 ^= 0x04;
	PF2 ^= 0x04;
	ADCstatus = 1;
	ADC_In89(ADCmail);
	PF2 ^= 0x04;
}

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 0x003FFFFF; 
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x07; 
	
	int32_t ADCmail[2] ={0,0};
	int32_t	ADCstatus = 0;
}

