#define SPEED 4
#define JUMP_HIGH 20
typedef struct
{
	SDL_Rect ground = { 0,0,16,16 };	//x,y - location in file, width, height
}element_t;

typedef struct
{
	int x;
	int y;
}pos_t;

typedef struct
{
	SDL_Rect stand_l = { 181, 0, 13, 16 };
	SDL_Rect stand_r = { 211, 0, 13, 16 };
	/*SDL_Rect go_l
	SDL_Rect go_r
	SDL_Rect jump_l
	SDL_Rect jump_r*/
	SDL_Rect *curr_frame;
	pos_t pos;
}mario_t;
