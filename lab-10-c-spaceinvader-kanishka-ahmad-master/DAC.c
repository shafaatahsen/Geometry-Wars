#include <stdint.h>
#include "tm4c123gh6pm.h"

uint8_t x = 0;
void DAC_Init(void){
 SYSCTL_RCGCGPIO_R |= 0x08;//clock
	x++; 
	x++; 
 while((SYSCTL_RCGCGPIO_R&0X08)==0){};//delay
	 
	GPIO_PORTD_DIR_R |= 0xFF;
	GPIO_PORTD_DEN_R |= 0xFF;
	GPIO_PORTD_AFSEL_R &= ~0xFF;
	GPIO_PORTD_AMSEL_R &= ~0xFF;
		
	
}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63
// Output: none
void DAC_Out(uint32_t data)
{  
		GPIO_PORTD_DATA_R = data&0xF+ ((data&0x30)<<2);
}


