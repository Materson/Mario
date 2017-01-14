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

void newGame(mario_t &mario, level_t &level, double &time)
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

int load_map( mario_t &mario, block_t block, level_t &level)
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

	//fill map
	while (!feof(file))
	{
		for (int i = 0; i < level.h; i++)
		{
			for (int j = 0; j < level.w; j++)
			{
				fscanf(file, "%d", &level.map[i][j]);
			}
		}
	}
	fclose(file);

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

void load_level(SDL_Surface *screen, level_t &level, mario_t &mario, block_t block)
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

	if (level.time - time > 0)
	{
		if (level.map[y][left_corner] == NOTHING && level.map[y][right_corner] == NOTHING)
		{
			if (mario.start_jump > 0 && !mario.end_jump && decimal % JUMP_SPEED == 0)
			{
				mario.pos.y--;
				mario.start_jump++;
				if (mario.start_jump == JUMP_HIGH) mario.end_jump = 1;
			}

		}
	}

	//fall down
	y = (mario.pos.y + mario.curr_frame->h - level.start_y) / block.ground.h;
	if (decimal % MOVE_SPEED == 0)
	{
		int right_corner = (mario.pos.x + level.start_x + mario.curr_frame->w - 1) / block.ground.w;
		int left_corner = (mario.pos.x + level.start_x) / block.ground.h;

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
	}
}

void move(mario_t &mario, level_t level, block_t block, double time)
{
	int x, y;
	int decimal = (time * 100) / 1;
	int mario_bottom = mario.pos.y + mario.curr_frame->h;
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
		switch (mario.status)
		{
		case RIGHT:
			if (mario.pos.x + mario.curr_frame->w == SCREEN_WIDTH) break;

			if (decimal % MOVE_SPEED == 0)
			{
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
			}
			break;
		case LEFT:
			if (mario.pos.x - 1 < 0) break;

			if (decimal % MOVE_SPEED == 0)
			{
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
			}
			break;
		}
	}
}

void camera(mario_t &mario, level_t &level, block_t block)
{
	//right move
	if (level.w * block.ground.w - level.start_x - 1 != SCREEN_WIDTH)
	{
		if (mario.pos.x >= (2 * SCREEN_WIDTH) / 3)
		{
			level.start_x++;
			mario.pos.x -=  1;
		}
	}
	//left move
	if (level.start_x != 0)
	{
		if (mario.pos.x == (1 * SCREEN_WIDTH) / 3)
		{
			level.start_x--;
			mario.pos.x += 1;
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

	if (!load_map(mario, block, level))
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

		distance += 10 * delta;

		SDL_FillRect(screen, NULL, niebieski);
		load_level(screen, level, mario, block);

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

		if (level.curr >= level.all && mario.status == META)
		{
			worldTime == level.time;
			sprintf(text, "Przeszedles wszystkie etapy!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
			sprintf(text, "Wcisnij n, aby zaczac od nowa lub ESC aby zakonczyc");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 10, text, charset);
		}
		else if (level.time - worldTime <= 0)
		{
			if (mario.lifes > 0)
			{
				mario.lifes--;
				if (mario.lifes != 0) newGame(mario, level, worldTime);
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
						if (level.curr > level.all)
						{
							level.curr = 1;
							if (!load_map(mario, block, level))
							{
								printf("Blad tworzenia mapy");
								return 0;
							}
						}
						newGame(mario, level, worldTime);
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

					if(!mario.key.left && !mario.key.right && !mario.key.up && mario.status != META)
						mario.status = STAND;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};

		
		jump(mario, level, block, worldTime);
		move(mario, level, block, worldTime);
		camera(mario, level, block);

		if (mario.status == META && level.curr <= level.all)
		{
			if(level.curr <= level.all)
				level.curr++;
			if (level.curr <= level.all)
			{
				if (!load_map(mario, block, level))
				{
					printf("Blad tworzenia mapy");
					return 0;
				}
			}
			newGame(mario, level, worldTime);
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
