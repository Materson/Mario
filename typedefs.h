#define SCREEN_WIDTH	740
#define SCREEN_HEIGHT	480

#define MOVE_SPEED 5
#define JUMP_HIGH 50
#define JUMP_SPEED 4

typedef enum
{
	NOTHING,
	MARIO,
	GROUND,
	PLATFORM
}element_t;

typedef struct
{
	SDL_Rect ground = { 0,0,16,16 };	//x,y - location in file, width, height
	SDL_Rect platform = { 16,0,16,16 };
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
	SDL_Rect jump_l = { 29, 0, 17, 16 };
	SDL_Rect jump_r =  { 359, 0, 17, 16 };
	SDL_Rect *curr_frame;
	pos_t pos;
	pos_t start;
	int start_jump = 0, end_jump = 0;
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
}level_t;
