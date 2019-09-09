#include <stdint.h>
#include <stdbool.h>

typedef struct Player player;
typedef struct asteroid asteroid;
typedef struct shot shot;

struct Player{
	int x_pos;
	int y_pos;
	double x_real;
	double y_real;
	int dirx;
	int diry;
	int lives;
	int score;
	bool hit;
};

struct asteroid{
	int size;
	int x;
	int y;
	int width;
	int height;
	bool hit;
};

struct shot{
	int x_pos;
	int y_pos;
	double x_real;
	double y_real;
	double dirx;
	double diry;
	double dir;
	bool hit;
};

