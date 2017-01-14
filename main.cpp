#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<malloc.h>

extern "C" {
//#ifdef BIT64
//#include"./sdl64/include/SDL.h"
//#include"./sdl64/include/SDL_main.h"
//#else
#include"./sdl/include/SDL.h"
#include"./sdl/include/SDL_main.h"
//#endif
}
#include"typedefs.h"



// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

void DrawElement(SDL_Surface *screen, int x, int y, SDL_Rect element, SDL_Surface *source)
{
	SDL_Rect d;
	d.w = element.w;
	d.h = element.h;
	d.x = x;
	d.y = y;
	SDL_BlitSurface(source, &element, screen, &d);
}


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void newGame(mario_t &mario, level_t &level, monster_t &monster, double &time)
{
	if (mario.lifes <= 0)
	{
		mario.lifes = 3;
	}
	time = 0;
	mario.pos.x = mario.start.x;
	mario.pos.y = mario.start.y;
	mario.start_jump = 0;
	mario.end_jump = 0;
	level.start_x = 0;
	mario.curr_frame = &mario.stand_r;
	if(level.curr <= level.all)
		mario.status = STAND;
	for (int i = 0; i < monster.num; i++)
	{
		monster.info[i].turn = LEFT;
		monster.info[i].pos.x = monster.info[i].start.x;
		monster.info[i].pos.y = monster.info[i].start.y;
	}
}

char* file_name(int level)
{
	int length = strlen(MAP_NAME);
	char *name;
	name = (char*)malloc((length + 6) * sizeof(char));
	strcpy(name, MAP_NAME);
	char number[3];
	sprintf(number, "%02d", level);
	strcat(strcat(name, number), ".map");

	return name;
}

int load_map( mario_t &mario, block_t block, monster_t &monster, level_t &level)
{
	char *name = file_name(level.curr);
	FILE *file = fopen(name, "r");
	free(name);
	if (file == NULL)
	{
		printf("Blad otwarcia pliku");
		return 0;
	}

	fscanf(file, "%d", &level.w);
	fscanf(file, "%d", &level.h);
	fscanf(file, "%d", &level.time);

	//create map
	level.map = (element_t **)malloc(level.h*level.w * sizeof(element_t));
	if (level.map == NULL)
	{
		printf("Blad przyznania pamieci 1\n");
		level.error = 1;
		return 0;
	}
	for (int i = 0; i<level.h; i++)
	{
		level.map[i] = (element_t *)malloc(level.w * sizeof(element_t));
		if (level.map[i] == NULL)
		{
			printf("Blad przyznania pamieci 2\n");
			level.error = 1;
			return 0;
		}
	}

	//fill map and count monsters
	int monster_num = 0;
	while (!feof(file))
	{
		for (int i = 0; i < level.h; i++)
		{
			for (int j = 0; j < level.w; j++)
			{
				fscanf(file, "%d", &level.map[i][j]);
				if (level.map[i][j] == MONSTER)
				{
					monster_num++;
				}
			}
		}
	}
	fclose(file);
	monster.num = monster_num;

	//create monsters
	monster.info = (monster_info*)malloc(monster.num * sizeof(monster_info));
	if (monster.info == NULL)
	{
		printf("Blad tworzenia potworow\n");
		level.error = 1;
		return 0;
	}
	//monsters start position
	int k = 0;
	for (int i = 0; i < level.h; i++)
	{
		for (int j = 0; j < level.w; j++)
		{
			if (level.map[i][j] == MONSTER)
			{
				monster.info[k].start.x = j*block.ground.w + 1;
				monster.info[k].start.y = SCREEN_HEIGHT - (level.h - i)*block.ground.h;
				monster.info[k].pos.x = monster.info[k].start.x;
				monster.info[k].pos.y = monster.info[k].start.y;
				monster.info[k].turn = LEFT;
				level.map[i][j] = NOTHING;
				k++;
			}
		}
	}

	level.start_y = SCREEN_HEIGHT - (level.h * block.ground.h);
	level.start_x = 0;

	//mario start position
	for (int i = 0; i < level.h; i++)
	{
		for (int j = 0; j < level.w; j++)
		{
			if (level.map[i][j] == MARIO)
			{
				mario.start.x = j*block.ground.w + 1;
				mario.start.y = SCREEN_HEIGHT - (level.h - i)*block.ground.h;
				mario.pos.x = mario.start.x;
				mario.pos.y = mario.start.y;
				level.map[i][j] = NOTHING;
				return 1;
			}
		}
		//if mario doesn't exist
		if (i == level.h - 1)
		{
			mario.error = 1;
			mario.pos.x = 0;
			mario.pos.y = SCREEN_HEIGHT - block.ground.h - 1;
			mario.start.x = 0;
			mario.start.y = SCREEN_HEIGHT - block.ground.h - 1;
		}
	}


	return 1;
}

void load_level(SDL_Surface *screen, level_t level, mario_t mario, block_t block, monster_t monster)
{
	int start_x = 0, start_y = level.start_y;

	//draw map
	for (int i = 0; i < level.h; i++)
	{
		start_x = 0;
		for (int j = 0; j < level.w; j++)
		{
			switch (level.map[i][j])
			{
			case GROUND:
				DrawElement(screen, start_x - level.start_x, start_y, block.ground, block.sprite);
				break;
			case PLATFORM:
				DrawElement(screen, start_x - level.start_x, start_y, block.platform, block.sprite);
				break;
			case STAR:
				DrawElement(screen, start_x - level.start_x, start_y, block.star, block.sprite);
				break;
			}
			start_x += block.ground.w;
		}
		start_y += block.ground.h;
	}
	//draw monsters
	for (int i = 0; i < monster.num; i++)
	{
		if (monster.info[i].turn != MONSTER_DIE)
		{
			DrawElement(screen, monster.info[i].pos.x, monster.info[i].pos.y, monster.go, monster.sprite);
		}
	}

	//draw mario
	DrawElement(screen, mario.pos.x, mario.pos.y, *mario.curr_frame, mario.sprite);
}

int level_number()
{
	int i = 1;
	char *name = file_name(i);
	while (FILE *file = fopen(name, "r"))
	{
		fclose(file);
		i++;
		free(name);
		name = file_name(i);
	}
	free(name);

	return i - 1;
}

void jump(mario_t &mario, level_t level, block_t block, double time)
{
	int decimal = (time * 100) / 1;
	int y = (mario.pos.y - level.start_y - 1) / block.platform.h;
	int left_corner = (mario.pos.x + level.start_x) / block.platform.w;
	int right_corner = (mario.pos.x + level.start_x + mario.curr_frame->w - 1) / block.ground.w;

	if (y < 0)
	{
		y = 0;
	}

	if (level.map[y][left_corner] == NOTHING && level.map[y][right_corner] == NOTHING)
	{
		if (mario.start_jump > 0 && !mario.end_jump && decimal % JUMP_SPEED == 0)
		{
			mario.pos.y--;
			mario.start_jump++;
			if (mario.start_jump == JUMP_HIGH) mario.end_jump = 1;
		}

	}
	else 
	{
		mario.end_jump = 1;
	}
	//touch top star
	if (level.map[y][left_corner] == STAR || level.map[y][right_corner] == STAR)
	{
		mario.status = META;
	}
	

	//fall down
	y = (mario.pos.y + mario.curr_frame->h - level.start_y) / block.ground.h;
	if (y == level.h)
		y = level.h - 1;
	if (decimal % MOVE_SPEED == 0 && mario.status != FALL_OUT_DIE)
	{
		right_corner = (mario.pos.x + level.start_x + mario.curr_frame->w - 1) / block.ground.w;
		left_corner = (mario.pos.x + level.start_x) / block.ground.h;

		if (level.map[y][left_corner] == NOTHING && level.map[y][right_corner] == NOTHING)
		{
			if (mario.end_jump == 1 || mario.start_jump == 0)
			{
				mario.end_jump = 1;
				mario.pos.y++;
			}
		}
		else
		{
			mario.start_jump = 0;
			mario.end_jump = 0;
			if (mario.curr_frame == &mario.jump_l)
			{
				mario.curr_frame = &mario.stand_l;
			}
			else if (mario.curr_frame == &mario.jump_r)
			{
				mario.curr_frame = &mario.stand_r;
			}
		}
		//touch bottom start
		if (level.map[y][left_corner] == STAR || level.map[y][right_corner] == STAR)
		{
			mario.status = META;
		}
	}

}

int move(mario_t &mario, level_t &level, monster_t &monster, block_t block, double &time)
{
	int x;
	int decimal = (time * 100) / 1;
	int mario_bottom = mario.pos.y + mario.curr_frame->h;	//to check fall out and jump on the block
	int mario_right = mario.pos.x + mario.curr_frame->w -1;

	int mario_down = (mario.pos.y + mario.curr_frame->h - 1 - level.start_y) / block.ground.h;
	int mario_up = (mario.pos.y - level.start_y) / block.ground.h;

	int block_up_wall = level.start_y + (mario_down*block.platform.w);

	//mario to high
	if (mario_down < 0)
	{
		mario_down = 0;
	}
	if (mario_up < 0)
	{
		mario_up = 0;
	}

	//mario fall out
	if (mario_bottom >= SCREEN_HEIGHT)
	{
		if (mario.lifes > 0)
		{
			mario.lifes--;
			if (mario.lifes != 0) newGame(mario, level, monster, time);
			return 0;
		}
		if(mario.lifes == 0)
		{
			mario.status = FALL_OUT_DIE;
			return 0;
		}
	}

	x = ((mario.pos.x + level.start_x + mario.curr_frame->w + 1) / block.ground.w);
	// if mario penetrade block at right
	if (level.map[mario_down][x] != NOTHING)
	{
		int element_left_wall = (x*block.platform.w) - level.start_x;
		while (mario_right >= element_left_wall)
		{
			mario.pos.x--;
			mario_right--;
		}
	}

	if (level.time - time > 0)
	{
		if (decimal % MOVE_SPEED == 0)
		{
			switch (mario.status)
			{
			case RIGHT:
				if (mario.pos.x + mario.curr_frame->w == SCREEN_WIDTH) break;

				x = ((mario.pos.x + level.start_x + mario.curr_frame->w) / block.ground.w);
				if (level.map[mario_up][x] == NOTHING && level.map[mario_down][x] == NOTHING)
				{
					mario.pos.x++;
					break;
				}
				else if (mario_bottom < block_up_wall)
				{
					mario.pos.x++;
					break;
				}

				if (level.map[mario_up][x] == STAR || level.map[mario_down][x] == STAR)
				{
					mario.status = META;
					break;
				}
				break;
			case LEFT:
				if (mario.pos.x - 1 < 0) break;

				x = (mario.pos.x + level.start_x - 1) / block.ground.w;
				if (level.map[mario_up][x] == NOTHING && level.map[mario_down][x] == NOTHING)
				{
					mario.pos.x--;
					break;
				}
				else
				{
					if (mario_bottom < block_up_wall)
					{
						mario.pos.x--;
						break;
					}
				}

				if (level.map[mario_up][x] == STAR || level.map[mario_down][x] == STAR)
				{
					mario.status = META;
					break;
				}
				break;
			}
		}
	}
	return 0;
}

void camera(mario_t &mario, level_t &level, monster_t monster, block_t block)
{
	//right move
	if (level.w * block.ground.w - level.start_x - 1 != SCREEN_WIDTH)
	{
		if (mario.pos.x >= (2 * SCREEN_WIDTH) / 3)
		{
			level.start_x++;
			mario.pos.x--;
			for (int i = 0; i < monster.num; i++)
			{
				if (monster.info[i].turn != MONSTER_DIE)
				{
					monster.info[i].pos.x--;
				}
			}
		}
	}
	//left move
	if (level.start_x != 0)
	{
		if (mario.pos.x <= (1 * SCREEN_WIDTH) / 3)
		{
			level.start_x--;
			mario.pos.x++;
			for (int i = 0; i < monster.num; i++)
			{
				if (monster.info[i].turn != MONSTER_DIE)
				{
					monster.info[i].pos.x++;
				}
			}
		}
	}
}

void save(mario_t mario, level_t level, monster_t monster, double time)
{
	FILE *file = fopen("save.txt", "w");
	fprintf(file, "%d %d %.2f %d %d %d %d %d", level.curr, level.start_x, time, mario.pos.x, mario.pos.y, mario.lifes, mario.start_jump, mario.end_jump);
	fclose(file);
}

int load(mario_t &mario, level_t &level, block_t block, monster_t &monster, double &time)
{
	int curr_level = level.curr;
	FILE *file = fopen("save.txt", "r");
	if (file == NULL)
	{
		printf("Brak pliku zapisu");
		return 0;
	}

	fscanf(file, "%d", &level.curr);
	if (!load_map(mario, block, monster, level))
	{
		printf("Blad tworzenia mapy");
		level.curr = curr_level;
		return 0;
	}
	fscanf(file, "%d %lf %d %d %d %d %d",&level.start_x, &time, &mario.pos.x, &mario.pos.y, &mario.lifes, &mario.start_jump, &mario.end_jump);
	fclose(file);

	return 1;
}

void monster_move(monster_t &monster, level_t level, block_t block, double time)
{
	int decimal = (time * 100) / 1;
	int x;
	for (int i = 0; i < monster.num; i++)
	{
		int monster_down = (monster.info[i].pos.y + monster.go.h - 1 - level.start_y) / block.ground.h;
		int monster_up = (monster.info[i].pos.y - level.start_y) / block.ground.h;
		if (monster_down == level.h)
			monster_down = level.h - 1;
		if (decimal % MONSTER_SPEED == 0 && monster.info[i].turn != MONSTER_DIE)
		{
			switch (monster.info[i].turn)
			{
			case RIGHT:
				x = ((monster.info[i].pos.x + level.start_x + monster.go.w) / block.ground.w);
				if (level.map[monster_down][x] == NOTHING)
				{
					monster.info[i].pos.x++;
					break;
				}
				else
				{
					monster.info[i].turn = LEFT;
				}
				break;
			case LEFT:
				x = ((monster.info[i].pos.x + level.start_x) / block.ground.w);
				if (level.map[monster_down][x] == NOTHING && monster.info[i].pos.x - 1 >= 0)
				{
					monster.info[i].pos.x--;
					break;
				}
				else
				{
					monster.info[i].turn = RIGHT;
				}
				break;
			}

			//fall down
			int y = (monster.info[i].pos.y + monster.go.h - level.start_y) / block.ground.h;
			if (y == level.h)
				y = level.h - 1;

			int right_corner = (monster.info[i].pos.x + level.start_x + monster.go.w - 1) / block.ground.w;
			int left_corner = (monster.info[i].pos.x + level.start_x) / block.ground.h;
			if (level.map[y][left_corner] == NOTHING && level.map[y][right_corner] == NOTHING)
			{
				monster.info[i].pos.y++;
			}

			if (monster.info[i].pos.y == SCREEN_HEIGHT - block.ground.h)
			{
				monster.info[i].turn = MONSTER_DIE;
			}
		}				
	}
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	block_t block;
	mario_t mario;
	monster_t monster;


	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe≥noekranowy
	/*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	                                 &window, &renderer);
*/
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);

	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2014");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	block.sprite = SDL_LoadBMP("./block_sprite.bmp");
	if (block.sprite== NULL) {
		printf("SDL_LoadBMP(block_sprite.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	monster.sprite = SDL_LoadBMP("./monster_sprite.bmp");
	if (block.sprite == NULL) {
		printf("SDL_LoadBMP(monster_sprite.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	mario.sprite = SDL_LoadBMP("./mario_sheet.bmp");
	if (mario.sprite == NULL) {
		printf("SDL_LoadBMP(mario_sheet.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};


	//eti = SDL_LoadBMP("./eti.bmp");
	//if(eti == NULL) {
	//	printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
	//	SDL_FreeSurface(charset);
	//	SDL_FreeSurface(screen);
	//	SDL_DestroyTexture(scrtex);
	//	SDL_DestroyWindow(window);
	//	SDL_DestroyRenderer(renderer);
	//	SDL_Quit();
	//	return 1;
	//	};

	//if (eti == NULL) {
	//	printf("SDL_LoadBMP(misc_sprites.bmp) error: %s\n", SDL_GetError());
	//	SDL_FreeSurface(charset);
	//	SDL_FreeSurface(screen);
	//	SDL_DestroyTexture(scrtex);
	//	SDL_DestroyWindow(window);
	//	SDL_DestroyRenderer(renderer);
	//	SDL_Quit();
	//	return 1;
	//};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x5C, 0x94, 0xFC);

	t1 = SDL_GetTicks();

	//map
	level_t level;
	level.all = level_number();
	if (level.all == 0)
	{
		printf("Brak plikow z mapami");
		return 0;
	}

	if (!load_map(mario, block, monster, level))
	{
		printf("Blad przy ladowaniu mapy");
		return 0;
	}

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;

	//rysowanie
	while(!quit) {
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna≥ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;
		if (level.time - worldTime <= 0)
			mario.status = TIME_DIE;

		distance += 10 * delta;

		SDL_FillRect(screen, NULL, niebieski);
		load_level(screen, level, mario, block, monster);

		//rysuj eti
		/*DrawSurface(screen, eti,
		            SCREEN_WIDTH / 2 + sin(distance) * SCREEN_HEIGHT / 3,
			    SCREEN_HEIGHT / 2 + cos(distance) * SCREEN_HEIGHT / 3);*/


//		DrawScreen(screen, plane, ship, charset, worldTime, delta, vertSpeed);

		// naniesienie wyniku rysowania na rzeczywisty ekran
//		SDL_Flip(screen);

		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		//pass all levels
		if (level.curr >= level.all && mario.status == META)
		{
			worldTime == level.time;
			sprintf(text, "Przeszedles wszystkie etapy!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
			sprintf(text, "Wcisnij n, aby zaczac od nowa lub ESC aby zakonczyc");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 15, text, charset);
		}
		//end time
		else if (mario.status == TIME_DIE)	
		{
			if (mario.lifes > 0)
			{
				mario.lifes--;
				if (mario.lifes != 0) newGame(mario, level, monster, worldTime);
			}
			else
			{
				sprintf(text, "Pozostaly czas = 0 s  %.0lf klatek / s", fps);
				int text_len = strlen(text);	//remeber to draw lifes in good position
				DrawElement(screen, screen->w / 2 - strlen(text) * 8 / 2 - mario.heart.w - 40, 10, mario.heart, mario.sprite);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
				sprintf(text, "x%d", mario.lifes);
				DrawString(screen, screen->w / 2 - text_len * 8 / 2 - mario.heart.w - 25, 10, text, charset);
				sprintf(text, "KONIEC CZASU");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
				sprintf(text, "Wcisnij n, aby rozpoczac nowa gre lub ESC aby zakonczyc");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
			}
		}
		else if (mario.status == MONSTER_DIE)
		{

		}
		else if (mario.status == FALL_OUT_DIE)
		{
			sprintf(text, "x%d", mario.lifes);
			DrawElement(screen, screen->w / 2 - strlen(text) * 8 / 2 - mario.heart.w - 40, 10, mario.heart, mario.sprite);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 - mario.heart.w - 25, 10, text, charset);
			sprintf(text, "MARIO MIAL LEK WYSOKOSCI");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
			sprintf(text, "Wcisnij n, aby rozpoczac nowa gre lub ESC aby zakonczyc");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
		}
		else
		{
			// tekst informacyjny
			sprintf(text, "Pozostaly czas = %.1lf s  %.0lf klatek / s, level - %d/%d", level.time-worldTime, fps, level.curr, level.all);
			int text_len = strlen(text);	//remeber to draw lifes in good position
			DrawElement(screen, screen->w / 2 - strlen(text) * 8 / 2 - mario.heart.w - 40, 10, mario.heart, mario.sprite);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			sprintf(text, "x%d", mario.lifes);
			DrawString(screen, screen->w / 2 - text_len * 8 / 2 - mario.heart.w - 25, 10, text, charset);
			sprintf(text, "Esc - wyjscie, n - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		}




		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs≥uga zdarzeÒ (o ile jakieú zasz≥y)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_n)
					{
						if (level.curr > level.all || mario.lifes == 0)
						{
							level.curr = 1;
							mario.lifes = 3;
							if (!load_map(mario, block, monster, level))
							{
								printf("Blad tworzenia mapy");
								return 0;
							}
						}
						newGame(mario, level, monster, worldTime);
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						save(mario, level, monster, worldTime);
					}
					else if (event.key.keysym.sym == SDLK_l)
					{
						load(mario, level, block, monster, worldTime);
					}
					else if (event.key.keysym.sym == SDLK_RIGHT && level.curr <= level.all)
					{
						mario.key.right = 1;
						mario.status = RIGHT;

						if (mario.start_jump == 0)
						{
							mario.curr_frame = &mario.stand_r;
						}
						else
						{
							mario.curr_frame = &mario.jump_r;
						}
					}
					else if (event.key.keysym.sym == SDLK_LEFT  && level.curr <= level.all)
					{
						mario.key.left = 1;
						mario.status = LEFT;

						if (mario.start_jump == 0)
						{
							mario.curr_frame = &mario.stand_l;
						}
						else
						{
							mario.curr_frame = &mario.jump_l;
						}
					}
					else if (event.key.keysym.sym == SDLK_UP  && level.curr <= level.all)
					{
						if (mario.start_jump == 0)
						{
							mario.key.up = 1;
							mario.start_jump = 1;
							if (mario.curr_frame == &mario.stand_l)
							{
								mario.curr_frame = &mario.jump_l;
							}
							else if(mario.curr_frame == & mario.stand_r)
							{
								mario.curr_frame = &mario.jump_r;
							}

						}
					}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_UP)
						mario.key.up = 0;
					if (event.key.keysym.sym == SDLK_RIGHT)
					{
						mario.key.right = 0;
					}
					if (event.key.keysym.sym == SDLK_LEFT)
					{
						mario.key.left = 0;
					}

					if(!mario.key.left && !mario.key.right && !mario.key.up && (mario.status == LEFT || mario.status == RIGHT))
						mario.status = STAND;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};

		if (mario.status != FALL_OUT_DIE  && mario.status != MONSTER_DIE && mario.status != TIME_DIE)
		{
			move(mario, level, monster, block, worldTime);
			jump(mario, level, block, worldTime);
			camera(mario, level, monster, block);
			monster_move(monster, level, block, worldTime);
		}

		if (mario.status == META && level.curr <= level.all)
		{
			if(level.curr <= level.all)
				level.curr++;
			if (level.curr <= level.all)
			{
				if (!load_map(mario, block, monster, level))
				{
					printf("Blad tworzenia mapy");
					return 0;
				}
				newGame(mario, level, monster, worldTime);
			}
		}
		
		
		frames++;
		};

	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
