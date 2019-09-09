#include "EdgeInterrupts.h"
#include "tm4c123gh6pm.h"

int nop;
void EdgeCounter_Init(void){                          
  SYSCTL_RCGCGPIO_R |= 0x00000002; // (a) activate clock for port B
	nop= 0;  
	nop=1;
	nop=2;
  GPIO_PORTB_DIR_R &= ~0x4;    // (c) make PB2 in (built-in button)
  GPIO_PORTB_AFSEL_R &= ~0x4;  //     disable alt funct on PB2
  GPIO_PORTB_DEN_R |= 0x4;     //     enable digital I/O on PB2  
  GPIO_PORTB_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTB_PDR_R |= 0x4;     //     enable weak pull-up on PB2
  GPIO_PORTB_IS_R &= ~0x4;     // (d) PB2 is edge-sensitive
  GPIO_PORTB_IBE_R &= 0x4;    //     PB2 is not both edges
  GPIO_PORTB_IEV_R &= 0x4;    //     PB2 falling edge event
  GPIO_PORTB_ICR_R = 0x4;      // (e) clear flag4
  GPIO_PORTB_IM_R |= 0x4;      // (f) arm interrupt on PB2 *** No IME bit as mentioned in Book ***
  NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFF1FFF)|0x00006000; // (g) priority 5
  NVIC_EN0_R = 0x0000002;      // (h) enable interrupt 30 in NVIC
  EnableInterrupts();           // (i) Clears the I bit
}
void GPIOPortB_Handler(void){
	shotflag =1;
  GPIO_PORTB_ICR_R = 0x4;      // acknowledge flag4
}
