#define SCREEN_WIDTH	740
#define SCREEN_HEIGHT	480

#define MOVE_SPEED 4
#define JUMP_HIGH 25
#define JUMP_SPEED 4

#define MAP_NAME "map"

typedef enum
{
	NOTHING,
	MARIO,
	GROUND,
	PLATFORM,
	STAR
}element_t;

typedef enum
{
	STAND,
	LEFT,
	RIGHT,
	META
}status_t;

typedef struct
{
	int left = 0;
	int right = 0;
	int up = 0;
}key_t;

typedef struct
{
	SDL_Rect ground = { 0,0,16,16 };	//x,y - location in file, width, height
	SDL_Rect platform = { 16,0,16,16 };
	SDL_Rect check = { 32,0,16,16 };
	SDL_Rect star = { 48,0,16,16 };
	SDL_Surface *sprite;
}block_t;

typedef struct
{
	int x;
	int y;
}pos_t;

typedef struct
{
	SDL_Surface *sprite;
	SDL_Rect stand_l = { 181, 0, 12, 16 };
	SDL_Rect stand_r = { 211, 0, 12, 16 };
	SDL_Rect jump_l = { 29, 0, 15, 16 };
	SDL_Rect jump_r =  { 359, 0, 15, 16 };
	SDL_Rect heart = { 3, 3, 11, 9 };
	SDL_Rect *curr_frame = &stand_r;
	pos_t pos;
	pos_t start;
	key_t key;
	status_t status = STAND;
	int start_jump = 0, end_jump = 0;
	int lifes = 3;
	int error = 0;
	/*
	SDL_Rect go_l
	SDL_Rect go_r
	*/
}mario_t;

typedef struct
{
	int start_y, start_x, w, h;
	element_t **map;
	int error = 0;
	int time;
	int curr = 1, all;
}level_t;
