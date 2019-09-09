#include <stdint.h>
struct playa{
	uint32_t x;
	uint32_t y;
	uint32_t points;
	int lives;
	int speed;	
	int dirx;
	int diry;
};
typedef struct ball{
	int dirx;
	int diry;
	uint16_t x;
	uint16_t y;
	uint32_t count;
	int life;        //0 means shots are dead
}shots;



typedef struct playa player;


void playerinit(player *plyr);//initalizes player
void movplayer(player *playa);
void shoot(shots *shot, player *player1, int i);
void shotsinit(shots *shot, int i);
void shotmove(shots *shot, int i, int *t);


