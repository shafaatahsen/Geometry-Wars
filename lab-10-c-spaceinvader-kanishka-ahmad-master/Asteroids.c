//Asteroids.c 
//Ahmad and Kanishka 

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "ADC.h"
#include "PLL.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
#include "sound.h"
#include "DAC.h"
#include "images.h" 
#include "gamestruct.c" 
#include "EdgeInterrupts.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void Port_Init(void); 
void SysTick_Init(uint32_t period); 
void SysTick_Handler(void); 
void Player_Init(player *playa); 
void Player_Update(player *playa); 
void newasteroid(asteroid *ast, player p1);
void drawasteroid(asteroid *ast); 
void Player_Hit(player *playa);
void asteroidmove(asteroid asteroids[], player p1); 
void Player_Move(player *playa); 
void shothit(shot shot, asteroid asteroids[], int index);
void shotgo(shot shots[], player p1);
void shotmove(shot shots[]);
void Game_Over(player p1);

//*************************            Movement variables       *******************************************//

uint32_t ADCStatus =0;
uint32_t ADCXm;
uint32_t ADCYm;
uint32_t ADCXs;
uint32_t ADCYs;

uint32_t XYm[2];
uint32_t XYs[2];

//*************************            Asteroid Variables      *******************************************//

asteroid asteroids[50];
int astcnt = 0;
int spdast = 1;
int spd = 0;

//*************************           Bullet Variables          *******************************************//

shot shots[500];
int shotcnt;
int shotwait = 0;
uint32_t shotflag;

//*************************            Convert            *******************************************//

double ConvertX(uint32_t input){
		return ((double)input - 3048);
}

double ConvertY(uint32_t input){
		return (3000 - (double)input);
}


//**************************************************************************************************//
//*************************            Main              *******************************************//
//**************************************************************************************************//
int main(void){
//Initialize 
  PLL_Init(Bus80MHz);   // set system clock to 80 MHz
  Output_Init(); 
  ST7735_FillScreen(0x0000);   // set screen to black
  Delay100ms(10);		// delay .5 sec at 80 MHz
  Port_Init(); 
  ADC_InitXYmove(); 
	ADC_InitXYshoot(); 
	SysTick_Init(1000000);	//40 Hz
	srand(ADC0_SSFIFO2_R&0xFFF);
	EdgeCounter_Init();

	ST7735_DrawBitmap(16, 96, betterlogo, 96, 32);
	ST7735_SetCursor(4, 0);
	ST7735_OutString("Press to Begin");
	shotflag=0;

	
	while(!(GPIO_PORTF_DATA_R&0x10)){
	//do nothing 
	}

	DAC_Init();
		
	Timer0A_Init(&TimerTask); //sound
		
	ST7735_FillScreen(0x0000); 
	player p1;
	Player_Init(&p1);
	
	
  while(1){

		if (ADCStatus==1)
		{
			//if lives are zero then game over 
			if((p1.lives ==0) || (GPIO_PORTB_DATA_R&0x08)== 0x08)
				Game_Over(p1);
			
			ST7735_SetCursor(0,0);
			ST7735_OutString("Score:");
			LCD_OutDec(p1.score);
			ST7735_SetCursor(11,0);
			ST7735_OutString("  Lives:");
			LCD_OutDec(p1.lives);
			if((spd%70) == 0){
				int p = 0;
				while(p < spdast){
					asteroid ast;
					newasteroid(&ast, p1);
					p++;
				}
			}
			ADCStatus = 0;
			if(ADCXm > 3200 || ADCYm < 3100 || ADCYm > 3100 || ADCXm < 2900)
				Player_Move(&p1);
			if(spd%2 == 0)
				asteroidmove(asteroids, p1);
			Player_Hit(&p1);
			if(p1.hit){
				astcnt = 0;
				ST7735_FillScreen(0x0000); 
				Player_Update(&p1);
			}
			if(ADCXm > 3200 || ADCYm < 3100 || ADCYm > 3100 || ADCXm < 2900)

				if(shotflag==1){
					if(spd%15 == 0){
						shotgo(shots, p1);
						shotflag=0;
					}
				}
		
			shotmove(shots);
			spd++;
			if(spd%40 == 0)
				p1.score++;
			if(spd%700 == 0)
				spdast++;
			
			ST7735_FillRect(p1.x_pos, p1.y_pos - 16, 20, 16, 0x0000); 
			ST7735_DrawBitmap(p1.x_pos, p1.y_pos, Player, 20, 16);
			
			
			
		}
	}
}


//**************************************************************************************************//
//*************************             Player           *******************************************//
//**************************************************************************************************//

//the original position of the player 
void Player_Init(player *playa){
	playa->x_pos = 54; // Initialize to middle of screen
	playa->y_pos = 88;
	playa->x_real = 54;	//the player's origin 
	playa->y_real = 88;
	playa->dirx = 90; // setting the direction, foreward 
	playa->lives = 3; // starts with 3 lives
	playa->score = 0; // score is initially 0
	playa->hit = false; // initially player is not hit
	ST7735_DrawBitmap(playa->x_pos, playa->y_pos, Player, 20, 16);
}

void Player_Update(player *playa){
	playa->x_pos = 54; // Initialize to middle of screen
	playa->y_pos = 88;
	playa->x_real = 54;
	playa->y_real = 88;
	playa->dirx = 90; // Facing north (i.e. 90 deg)
	playa->hit = false;
	ST7735_DrawBitmap(playa->x_pos, playa->y_pos, Player, 20, 16);
}
void Player_Move(player *playa){
	playa->dirx =1;
	playa->diry = 1;
	if(ADCXm<1000){
		if((playa->x_pos -2)<-20){
			playa->x_pos =120;
			playa->dirx = -2;
		}
		else{
			playa->x_pos = (playa->x_pos -2);
			playa->dirx = -2;
		}
		
	}
	else if(ADCXm>3500){
		playa->x_pos = (playa->x_pos +2)%128;
		playa->dirx = 2;
	}
	
	if(ADCYm< 1000){
		if((playa->y_pos -2)<-10){
			playa->y_pos = 180;
			playa->diry = -2;
		}
		else{
			playa->y_pos = playa->y_pos -2; 
			playa->diry = -2;
		}		
	}
	else if(ADCYm>3300){
		playa->y_pos= (playa->y_pos +2)%180;
		playa->diry = 2;
	}
	ST7735_DrawBitmap(playa->x_pos, playa->y_pos, Player, 20, 16);
}

// if the player hits an asteroid
void Player_Hit(player *playa){
	for(int i = 0; i < astcnt; i++){
		if((playa->x_pos < (asteroids[i].x + asteroids[i].width))&& ((playa->x_pos + 20) > asteroids[i].x)&& (playa->y_pos > (asteroids[i].y - asteroids[i].height))&& ((playa->y_pos - 16) < asteroids[i].y))
		{
			playa->lives--;
			playa->hit = true;
			break;
		}
	}
}

//**************************************************************************************************//
//*************************            Asteroids         *******************************************//
//**************************************************************************************************//
//this spawns a new asteroid in a new place, also it spawns a randomly sized asteroid 

void newasteroid(asteroid *ast, player p1){
	
	//get a random number within 16, to get randomly sized asteroid 
	ast->size = rand()%16;
		if(ast->size < 4){
			ast->size = 0;
			ast->width = 9;
			ast->height = 9;
		}
		else if (ast->size < 8){
			ast->size = 1;
			ast->width = 12;
			ast->height = 12;
		}
		else if (ast->size < 12){
			ast->size = 2;
			ast->width = 16;
			ast->height = 16;
		}
		else{
			ast->size = 3;
			ast->width = 22;
			ast->height = 22;
		}
	//get random postion for the asteroids 
	ast->x = rand()%(128 - ast->width);
	ast->y = ast->width + ast->height + rand()%((160-ast->width)-ast->height);
		
	//if the amt of asteroids is 0 then 
	if(astcnt  == 0)
		while((ast->x < (p1.x_pos + 20))&& ((ast->x + ast->width) > p1.x_pos)&& (ast->y > (p1.y_pos - 16))&& ((ast->y - ast->height) < p1.y_pos))
		{
			ast->x = rand()%(128 - ast->width);
	    ast->y = ast->width + ast->height + rand()%((160-ast->width)-ast->height);
		}
	for(int i = 0; i < astcnt; i++){
		if((ast->x < (asteroids[i].x + asteroids[i].width))&& ((ast->x + ast->width) > asteroids[i].x)&& (ast->y > (asteroids[i].y - asteroids[i].height))&& ((ast->y - ast->height) < asteroids[i].y)
			)
		{
			ast->x = rand()%(128 - ast->width);
	    ast->y = ast->width + ast->height + rand()%((160-ast->width)-ast->height);
			i = 0;
		}
		if((ast->x < (p1.x_pos + 20))&& ((ast->x + ast->width) > p1.x_pos)&& (ast->y > (p1.y_pos - 16))&& ((ast->y - ast->height) < p1.y_pos))
		{
			ast->x = rand()%(128 - ast->width);
	    ast->y = ast->width + ast->height + rand()%((160-ast->width)-ast->height);
			i = 0;
		}
	}
	
	ast->hit = false;
	asteroids[astcnt] = *ast;
	astcnt++;
	drawasteroid(ast);
}

//draws a new asteroid based on the size 
void drawasteroid(asteroid *ast){
	switch(ast->size){
		case 0:
			ST7735_DrawBitmap(ast->x, ast->y, ast0, ast->width, ast->height);
			break;
		case 1:
			ST7735_DrawBitmap(ast->x, ast->y, ast1, ast->width, ast->height);			
			break;
		case 2:
			ST7735_DrawBitmap(ast->x, ast->y, ast2, ast->width, ast->height);
			break;
		case 3:
			ST7735_DrawBitmap(ast->x, ast->y, ast3, ast->width, ast->height);
			break;
	}
}
// asteroids move towards the player 
void asteroidmove(asteroid asteroids[], player p1){
	for(int i = 0; i < astcnt; i++){
		ST7735_FillRect(asteroids[i].x, (asteroids[i].y - asteroids[i].height), asteroids[i].width, asteroids[i].height+1, 0x0000); 
		
		if((p1.x_pos - asteroids[i].x) > 0 && (p1.y_pos - asteroids[i].y) > 0)
		{
			asteroids[i].x++;
			asteroids[i].y++;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x--;
					asteroids[i].y--;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) < 0 && (p1.y_pos - asteroids[i].y) < 0)
		{
			asteroids[i].x--;
			asteroids[i].y--;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x++;
					asteroids[i].y++;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) > 0 && (p1.y_pos - asteroids[i].y) == 0)
		{
			asteroids[i].x++;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x--;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) == 0 && (p1.y_pos - asteroids[i].y) > 0)
		{
			asteroids[i].y++;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].y--;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) < 0 && (p1.y_pos - asteroids[i].y) == 0)
		{
			asteroids[i].x--;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x++;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) == 0 && (p1.y_pos - asteroids[i].y) < 0)
		{
			asteroids[i].y--;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].y++;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) < 0 && (p1.y_pos - asteroids[i].y) > 0)
		{
			asteroids[i].x--;
			asteroids[i].y++;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x++;
					asteroids[i].y--;
					break;
				}
			}
		}
		
		if((p1.x_pos - asteroids[i].x) > 0 && (p1.y_pos - asteroids[i].y) < 0)
		{
			asteroids[i].x++;
			asteroids[i].y--;
			for(int j = 0; j < astcnt; j++){
				if(i==j){}
				else if((asteroids[i].x < (asteroids[j].x + asteroids[j].width))&& ((asteroids[i].x + asteroids[i].width) > asteroids[j].x)&& (asteroids[i].y > (asteroids[j].y - asteroids[j].height))&& ((asteroids[i].y - asteroids[i].height) < asteroids[j].y))
				{
					asteroids[i].x--;
					asteroids[i].y++;
					break;
				}
			}
		}
		
		drawasteroid(&asteroids[i]);
	}
}


//**************************************************************************************************//
//*************************            Shooting          *******************************************//
//**************************************************************************************************//

// checks if shot hit
void shothit(shot shot, asteroid asteroids[], int index){
	for(int i = 0; i < astcnt; i++){
		if((shot.x_pos < (asteroids[i].x + asteroids[i].width))
		&& ((shot.x_pos + 20) > asteroids[i].x)
		&& (shot.y_pos > (asteroids[i].y - asteroids[i].height))
		&& ((shot.y_pos - 16) < asteroids[i].y)
			)
		{
			shot.hit = true;
			ST7735_FillRect(asteroids[i].x, (asteroids[i].y - asteroids[i].height), asteroids[i].width, asteroids[i].height+1, 0x0000);
			ST7735_FillRect(shot.x_pos, shot.y_pos - 5, 5, 6, 0x0000); 
			for(int j = i; j< astcnt; j++) 
			{ 
				asteroids[j] = asteroids[j + 1]; //Makes all the array indexes equal the number after them
			} 
			astcnt--;
			for(int j = index; j< shotcnt; j++) 
			{ 
				shots[j] = shots[j + 1]; //Makes all the array indexes equal the number after them
			} 
			shotcnt--;
			break;
		}
	}
}

// new shot
void shotgo(shot shots[], player p1){
	shots[shotcnt].dirx = p1.dirx;
	shots[shotcnt].diry = p1.diry;
	if(p1.dirx >0){
		shots[shotcnt].x_real = p1.x_pos + p1.dirx+10;
	}
	else{
		shots[shotcnt].x_real = p1.x_pos + p1.dirx -10;
	}
	
	if(p1.diry<0){
		shots[shotcnt].y_real = p1.y_pos  + p1.diry	-16;
	}
	else{
		shots[shotcnt].y_real = p1.y_pos  + p1.diry+10;
	}

	shots[shotcnt].x_pos = round(shots[shotcnt].x_real);
	shots[shotcnt].y_pos = round(shots[shotcnt].y_real);
	if(shots[shotcnt].x_pos < 0)
		shots[shotcnt].x_pos = 0;
	if((shots[shotcnt].y_pos - 16) <= 10)
		shots[shotcnt].y_pos = 26;
  if((shots[shotcnt].x_pos + 20) > 128)
		shots[shotcnt].x_pos = 108;
	if((shots[shotcnt].y_pos) > 160)
		shots[shotcnt].y_pos = 160;
	ST7735_DrawBitmap(shots[shotcnt].x_pos, shots[shotcnt].y_pos, ball, 5, 5);
	shotcnt++;
}

// Move shots in direction
void shotmove(shot shots[]){

	for(int i = 0; i < shotcnt; i++){
		ST7735_FillRect(shots[i].x_pos, shots[i].y_pos - 5, 6, 6, 0x0000); 
		shots[i].x_real = shots[i].x_pos + 3*shots[i].dirx;
		shots[i].y_real = shots[i].y_pos + 3*shots[i].diry;
		shots[i].x_pos = round(shots[i].x_real);
		shots[i].y_pos = round(shots[i].y_real);
		shothit(shots[i], asteroids, i);
		ST7735_DrawBitmap(shots[i].x_pos, shots[i].y_pos, ball, 5, 5);
	}

}



//**************************************************************************************************//
//*************************            Game Over         *******************************************//
//**************************************************************************************************//
//the game screen 
void Game_Over(player p1){
	DisableInterrupts();
	ST7735_FillScreen(0x0000);
	while(1){
		ST7735_SetCursor(7,0);
		ST7735_OutString("Game Over");
		ST7735_SetCursor(3,8);
		ST7735_OutString("Final Score:");
		LCD_OutDec(p1.score);
		//if((GPIO_PORTF_DATA_R&0x10) == 0x10) //start again 
			//main(); 
	}
}


//**************************************************************************************************//
//*************************            Delay          *******************************************//
//**************************************************************************************************//
// delay function 
void Delay100ms(uint32_t count){
	uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}


//**************************************************************************************************//
//*************************            Systick           *******************************************//
//**************************************************************************************************//
//the systick init,sets the period as the reload value 
void SysTick_Init(uint32_t period)
{
  NVIC_ST_CTRL_R = 0;         									// disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;									// reload value
  NVIC_ST_CURRENT_R = 0;     									  // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x0007; 											// enable SysTick with core clock and interrupts

}

//handler, sets up the arrays for the adc
void SysTick_Handler(void)
{
	GPIO_PORTF_DATA_R ^= 0x02;                      // toggle heartbeat
	GPIO_PORTF_DATA_R ^= 0x02;                      // toggle heartbeat
	XYmove(XYm);
	XYshoot(XYs);
	ADCXm = XYm[0];
	ADCYm = XYm[1];
	
	ADCXs = XYs[0];
	ADCYs = XYs[1];
	ADCStatus = 1;
	GPIO_PORTF_DATA_R ^= 0x02;                      // toggle heartbeat
}


//**************************************************************************************************//
//*************************           Port Init          *******************************************//
//**************************************************************************************************//
void Port_Init(void){
	volatile long delay; 
	SYSCTL_RCGCGPIO_R |= 0x000000022;          		// 1) activate clock for Port F, and port B
  delay = SYSCTL_RCGCGPIO_R;                     //  wait for clock to initialize
	GPIO_PORTF_DIR_R = 0x0E;                    // 2) make PF1, PF2, PF3 output, PF4 Input
	GPIO_PORTF_AFSEL_R = 0x00;                  // 3) regular port function
	GPIO_PORTF_DEN_R = 0x1E;                    // 4) enable PF1, PF2, PF3, PF4
	
	GPIO_PORTB_DIR_R &= ~0x0C;  
	GPIO_PORTB_DEN_R |= 0x0C; 
	
}


