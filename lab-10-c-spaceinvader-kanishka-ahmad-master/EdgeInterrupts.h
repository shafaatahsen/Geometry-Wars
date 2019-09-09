#include <stdint.h>


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void EdgeCounter_Init(void);
void GPIOPortB_Handler(void);

extern uint32_t shotflag;

