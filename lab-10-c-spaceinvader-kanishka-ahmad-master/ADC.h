// put prototypes for public functions, explain what it does
// put your names here, date
// ADC.h
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0

// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

// ADC initialization function 
// Input: none
// Output: none
#include <stdint.h>
void ADC_InitXYmove(void);

// Output: none
void ADC_InitXYshoot(void);
//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion

void XYmove(uint32_t data[2]);

void XYshoot(uint32_t data[2]);
