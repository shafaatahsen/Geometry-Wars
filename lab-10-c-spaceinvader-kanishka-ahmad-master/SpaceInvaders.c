// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 3/6/2015 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "ST7735.h"
#include "PLL.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
#include "sound.h"
#include "DAC.h"
#include "images.h" 

#define PI 3.14159265

typedef struct Player player;
typedef struct Enemy Enemy;
typedef struct Missile Missile;

// Player struct
// variables:
//        int  x_pos: x-coordinate
//				int	 y_pos: y-coordinate
//				int  dir: angle of movement	(0 deg is y=0, ccw)
//				int  lives: # of lives
//				int  score: current score
//				bool hit: false = not hit, true = hit
struct Player{
	int x_pos;
	int y_pos;
	double x_real;
	double y_real;
	double dir;
	int lives;
	int score;
	bool hit;
};

// Enemy struct
// variables:
//				int type: 0-4 for each type of enemy
//        int  x_pos: x-coordinate
//				int	 y_pos: y-coordinate
//				int  dir: angle of movement	(0 deg is y=0, ccw)
//				int  width: width of enemy img
//				int  height: height of enemy img
//				bool hit: false = not hit, true = hit
struct Enemy{
	int size;
	int x_pos;
	int y_pos;
	int width;
	int height;
	bool hit;
};

struct Missile{
	int x_pos;
	int y_pos;
	double x_real;
	double y_real;
	double dir;
	bool hit;
};

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

void Player_Init(player *playa); // DONE
void Player_Update(player *playa); //DONE
void Enemy_Spawn(Enemy *enemy, player p1); // DONE
void Draw_Enemy(Enemy *enemy); // DONE
void Player_Hit(player *playa);
void Enemy_Move(Enemy enemies[], player p1); // trying to get hit detection b/w enemies
void Player_Move(player *playa); //DONE
void Missile_Hit(Missile missile, Enemy enemies[], int index);
void Missile_Fire(Missile missiles[], player p1);
void Missiles_Move(Missile missiles[]);
void Game_Over(player p1);

void PortF_Init(void){
	volatile long delay; 
	SYSCTL_RCGCGPIO_R |= 0x000000020;          		// 1) activate clock for Port F
  delay = SYSCTL_RCGCGPIO_R;                     //  wait for clock to initialize
	GPIO_PORTF_DIR_R = 0x0E;                    // 2) make PF1, PF2, PF3 output, PF4 Input
	GPIO_PORTF_AFSEL_R = 0x00;                  // 3) regular port function
	GPIO_PORTF_DEN_R = 0x1E;                    // 4) enable PF1, PF2, PF3, PF4
}
uint32_t DataXm;        // 12-bit ADC
uint32_t DataYm;
uint32_t DataXs;        // 12-bit ADC
uint32_t DataYs;

uint32_t PositionXm;    // 32-bit fixed-point 0.001 cm
uint32_t PositionYm;    // 32-bit fixed-point 0.001 cm
uint32_t PositionXs;    // 32-bit fixed-point 0.001 cm
uint32_t PositionYs;    // 32-bit fixed-point 0.001 cm

uint32_t ADCStatus;
uint32_t ADCXm;
uint32_t ADCYm;
uint32_t ADCXs;
uint32_t ADCYs;

uint32_t XYm[2];
uint32_t XYs[2];

void SysTick_Init(uint32_t period)
{
  NVIC_ST_CTRL_R = 0;         									// disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;									// reload value
  NVIC_ST_CURRENT_R = 0;     									  // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x0007; 											// enable SysTick with core clock and interrupts

}
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


double ConvertX(uint32_t input){
		return ((double)input - 2048);
}

double ConvertY(uint32_t input){
		return (2000 - (double)input);
}



// width = 20 x height = 16



Enemy enemies[70];
int num_enemies = 0;
int enemy_freq = 1;
int freq_ct = 0;

Missile missiles[500];
int num_missiles;
int missile_delay = 0;

int main(void){
//****** Initializations and stuff here **********
  PLL_Init(Bus80MHz);   // set system clock to 80 MHz
  Output_Init(); 
  ST7735_FillScreen(0x0000);   // set screen to black
  Delay100ms(5);		// delay .5 sec at 80 MHz
  PortF_Init(); 
  ADC_InitXYmove(); 
	ADC_InitXYshoot(); 
	SysTick_Init(2000000);	//40 Hz
	srand(ADC0_SSFIFO2_R&0xFFF);

	ST7735_DrawBitmap(0, 160, betterlogo, 96, 32);
	while(!(GPIO_PORTF_DATA_R&0x10)){}
	DAC_Init();
	Timer0A_Init(&TimerTask);
	ST7735_FillScreen(0x0000); 
	player p1;
	Player_Init(&p1);
  while(1){
		if (ADCStatus==1)
		{
			if(p1.lives ==0)
				Game_Over(p1);
			ST7735_SetCursor(0,0);
			ST7735_OutString("Score:");
			LCD_OutDec(p1.score);
			ST7735_SetCursor(0,100);
			ST7735_OutString("  Lives:");
			LCD_OutDec(p1.lives);
			if((freq_ct%70) == 0){
				int k = 0;
				while(k < enemy_freq){
					Enemy e;
					Enemy_Spawn(&e, p1);
					k++;
				}
			}
			ADCStatus = 0;
			if(ADCXm > 3200 || ADCYm < 3100 || ADCYm > 3100 || ADCXm < 2900)
				Player_Move(&p1);
			if(freq_ct%2 == 0)
				Enemy_Move(enemies, p1);
			Player_Hit(&p1);
			if(p1.hit){
				num_enemies = 0;
				ST7735_FillScreen(0x0000); 
				Player_Update(&p1);
			}
			if(ADCXm > 3200 || ADCYm < 3100 || ADCYm > 3100 || ADCXm < 2900)
				if(freq_ct%9 == 0)
					Missile_Fire(missiles, p1);
			Missiles_Move(missiles);
			freq_ct++;
			p1.score++;
			if(freq_ct%700 == 0)
				enemy_freq++;
			
			ST7735_FillRect(p1.x_pos, p1.y_pos - 16, 20, 16, 0x0000); 
			ST7735_DrawBitmap(p1.x_pos, p1.y_pos, Player, 20, 16);
			
		}
	}
}

// Initializes player in the middle of the screen facing north
void Player_Init(player *playa){
	playa->x_pos = 54; // Initialize to middle of screen
	playa->y_pos = 88;
	playa->x_real = 54;
	playa->y_real = 88;
	playa->dir = 90; // Facing north (i.e. 90 deg)
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
	playa->dir = 90; // Facing north (i.e. 90 deg)
	playa->hit = false;
	ST7735_DrawBitmap(playa->x_pos, playa->y_pos, Player, 20, 16);
}

// Spawns new enemy in a random location that is not already occupied
void Enemy_Spawn(Enemy *enemy, player p1){
	enemy->size = rand()%100;
		if(enemy->size < 25){
			enemy->size = 0;
			enemy->width = 9;
			enemy->height = 9;
		}
		else if (enemy->size < 50){
			enemy->size = 1;
			enemy->width = 12;
			enemy->height = 12;
		}
		else if (enemy->size < 75){
			enemy->size = 2;
			enemy->width = 16;
			enemy->height = 16;
		}
		else{
			enemy->size = 3;
			enemy->width = 22;
			enemy->height = 22;
		}
		
	enemy->x_pos = rand()%(128 - enemy->width);
	enemy->y_pos = enemy->width + enemy->height + rand()%((160-enemy->width)-enemy->height);
	if(num_enemies == 0)
		while((enemy->x_pos < (p1.x_pos + 20))
		&& ((enemy->x_pos + enemy->width) > p1.x_pos)
		&& (enemy->y_pos > (p1.y_pos - 16))
		&& ((enemy->y_pos - enemy->height) < p1.y_pos)
			)
		{
			enemy->x_pos = rand()%(128 - enemy->width);
			enemy->y_pos = enemy->width + enemy->height + rand()%((160-enemy->width)-enemy->height);
		}
	for(int i = 0; i < num_enemies; i++){
		if((enemy->x_pos < (enemies[i].x_pos + enemies[i].width))
		&& ((enemy->x_pos + enemy->width) > enemies[i].x_pos)
		&& (enemy->y_pos > (enemies[i].y_pos - enemies[i].height))
		&& ((enemy->y_pos - enemy->height) < enemies[i].y_pos)
			)
		{
			enemy->x_pos = rand()%(128 - enemy->width);
			enemy->y_pos = enemy->width + enemy->height + rand()%((160-enemy->width)-enemy->height);
			i = 0;
		}
		if((enemy->x_pos < (p1.x_pos + 20))
		&& ((enemy->x_pos + enemy->width) > p1.x_pos)
		&& (enemy->y_pos > (p1.y_pos - 16))
		&& ((enemy->y_pos - enemy->height) < p1.y_pos)
			)
		{
			enemy->x_pos = rand()%(128 - enemy->width);
			enemy->y_pos = enemy->width + enemy->height + rand()%((160-enemy->width)-enemy->height);
			i = 0;
		}
	}
	enemy->hit = false;
	enemies[num_enemies] = *enemy;
	num_enemies++;
	Draw_Enemy(enemy);
}

// Draws enemy based on type
void Draw_Enemy(Enemy *enemy){
	switch(enemy->size){
		case 0:
			ST7735_DrawBitmap(enemy->x_pos, enemy->y_pos, ast0, enemy->width, enemy->height);
			break;
		case 1:
			ST7735_DrawBitmap(enemy->x_pos, enemy->y_pos, ast1, enemy->width, enemy->height);
			break;
		case 2:
			ST7735_DrawBitmap(enemy->x_pos, enemy->y_pos, ast2, enemy->width, enemy->height);
			break;
		case 3:
			ST7735_DrawBitmap(enemy->x_pos, enemy->y_pos, ast3, enemy->width, enemy->height);
			break;
	}
}

void Player_Move(player *playa){
	ST7735_FillRect(playa->x_pos, playa->y_pos - 16, 20, 16, 0x0000); 
	playa->dir = atan2(ConvertY(ADCYm), ConvertX(ADCXm));
	playa->x_real = playa->x_pos + 2*cos(playa->dir);
	playa->y_real = playa->y_pos + 2*sin(playa->dir);
	playa->x_pos = round(playa->x_real);
	playa->y_pos = round(playa->y_real);
	if(playa->x_pos < 0)
		playa->x_pos = 0;
	if((playa->y_pos - 16) <= 10)
		playa->y_pos = 26;
  if((playa->x_pos + 20) > 128)
		playa->x_pos = 108;
	if((playa->y_pos) > 160)
		playa->y_pos = 160;
	ST7735_DrawBitmap(playa->x_pos, playa->y_pos, Player, 20, 16);
}

// Detects if player has hit an enemy
void Player_Hit(player *playa){
	for(int i = 0; i < num_enemies; i++){
		if((playa->x_pos < (enemies[i].x_pos + enemies[i].width))
		&& ((playa->x_pos + 20) > enemies[i].x_pos)
		&& (playa->y_pos > (enemies[i].y_pos - enemies[i].height))
		&& ((playa->y_pos - 16) < enemies[i].y_pos)
			)
		{
			playa->lives--;
			playa->hit = true;
			break;
		}
	}
}

// Moves all enemies towards the player
void Enemy_Move(Enemy enemies[], player p1){
	for(int i = 0; i < num_enemies; i++){
		ST7735_FillRect(enemies[i].x_pos, (enemies[i].y_pos - enemies[i].height), enemies[i].width, enemies[i].height+1, 0x0000); 
		if((p1.x_pos - enemies[i].x_pos) > 0 && (p1.y_pos - enemies[i].y_pos) > 0)
		{
			enemies[i].x_pos++;
			enemies[i].y_pos++;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos--;
					enemies[i].y_pos--;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) < 0 && (p1.y_pos - enemies[i].y_pos) < 0)
		{
			enemies[i].x_pos--;
			enemies[i].y_pos--;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos++;
					enemies[i].y_pos++;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) > 0 && (p1.y_pos - enemies[i].y_pos) == 0)
		{
			enemies[i].x_pos++;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos--;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) == 0 && (p1.y_pos - enemies[i].y_pos) > 0)
		{
			enemies[i].y_pos++;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].y_pos--;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) < 0 && (p1.y_pos - enemies[i].y_pos) == 0)
		{
			enemies[i].x_pos--;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos++;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) == 0 && (p1.y_pos - enemies[i].y_pos) < 0)
		{
			enemies[i].y_pos--;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].y_pos++;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) < 0 && (p1.y_pos - enemies[i].y_pos) > 0)
		{
			enemies[i].x_pos--;
			enemies[i].y_pos++;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos++;
					enemies[i].y_pos--;
					break;
				}
			}
		}
		if((p1.x_pos - enemies[i].x_pos) > 0 && (p1.y_pos - enemies[i].y_pos) < 0)
		{
			enemies[i].x_pos++;
			enemies[i].y_pos--;
			for(int j = 0; j < num_enemies; j++){
				if(i==j){}
				else if((enemies[i].x_pos < (enemies[j].x_pos + enemies[j].width))
		&& ((enemies[i].x_pos + enemies[i].width) > enemies[j].x_pos)
		&& (enemies[i].y_pos > (enemies[j].y_pos - enemies[j].height))
		&& ((enemies[i].y_pos - enemies[i].height) < enemies[j].y_pos)
			)
				{
					enemies[i].x_pos--;
					enemies[i].y_pos++;
					break;
				}
			}
		}
		Draw_Enemy(&enemies[i]);
	}
}

// Detects if missile has hit enemies or wall

void Missile_Hit(Missile missile, Enemy enemies[], int index){
	for(int i = 0; i < num_enemies; i++){
		if((missile.x_pos < (enemies[i].x_pos + enemies[i].width))
		&& ((missile.x_pos + 20) > enemies[i].x_pos)
		&& (missile.y_pos > (enemies[i].y_pos - enemies[i].height))
		&& ((missile.y_pos - 16) < enemies[i].y_pos)
			)
		{
			missile.hit = true;
			ST7735_FillRect(enemies[i].x_pos, (enemies[i].y_pos - enemies[i].height), enemies[i].width, enemies[i].height+1, 0x0000);
			ST7735_FillRect(missile.x_pos, missile.y_pos - 5, 5, 6, 0x0000); 
			for(int j = i; j< num_enemies; j++) 
			{ 
				enemies[j] = enemies[j + 1]; //Makes all the array indexes equal the number after them
			} 
			num_enemies--;
			for(int j = index; j< num_missiles; j++) 
			{ 
				missiles[j] = missiles[j + 1]; //Makes all the array indexes equal the number after them
			} 
			num_missiles--;
			break;
		}
	}
}

// Fires a new missile in direction indicated
void Missile_Fire(Missile missiles[], player p1){
	missiles[num_missiles].dir = atan2(ConvertY(ADCYs), ConvertX(ADCXs));
	missiles[num_missiles].x_real = (p1.x_pos + 10) + 15*cos(missiles[num_missiles].dir);
	missiles[num_missiles].y_real = (p1.y_pos - 8) + 15*sin(missiles[num_missiles].dir);
	missiles[num_missiles].x_pos = round(missiles[num_missiles].x_real);
	missiles[num_missiles].y_pos = round(missiles[num_missiles].y_real);
	if(missiles[num_missiles].x_pos < 0)
		missiles[num_missiles].x_pos = 0;
	if((missiles[num_missiles].y_pos - 16) <= 10)
		missiles[num_missiles].y_pos = 26;
  if((missiles[num_missiles].x_pos + 20) > 128)
		missiles[num_missiles].x_pos = 108;
	if((missiles[num_missiles].y_pos) > 160)
		missiles[num_missiles].y_pos = 160;
	ST7735_DrawBitmap(missiles[num_missiles].x_pos, missiles[num_missiles].y_pos, Missiles, 5, 5);
	num_missiles++;
}

// Moves all missiles on the board in the direction that they are facing
void Missiles_Move(Missile missiles[]){
	for(int i = 0; i < num_missiles; i++){
		ST7735_FillRect(missiles[i].x_pos, missiles[i].y_pos - 5, 5, 6, 0x0000); 
		missiles[i].x_real = missiles[i].x_pos + 3*cos(missiles[i].dir);
		missiles[i].y_real = missiles[i].y_pos + 3*sin(missiles[i].dir);
		missiles[i].x_pos = round(missiles[i].x_real);
		missiles[i].y_pos = round(missiles[i].y_real);
		Missile_Hit(missiles[i], enemies, i);
		ST7735_DrawBitmap(missiles[i].x_pos, missiles[i].y_pos, Missiles, 5, 5);
	}
}


// Displays the game over screen
void Game_Over(player p1){
	DisableInterrupts();
	ST7735_FillScreen(0x0000);
	while(1){
		ST7735_SetCursor(0,10);
		ST7735_OutString("Game Over");
		ST7735_SetCursor(10,20);
		ST7735_OutString("Final Score:");
		LCD_OutDec(p1.score);
	}
}

// You can use this timer only if you learn how it works
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}


