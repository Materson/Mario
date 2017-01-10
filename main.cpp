#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

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

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480


// narysowanie napisu txt na powierzchni screen, zaczynaj�c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj�ca znaki
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
// (x, y) to punkt �rodka obrazka sprite na ekranie
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


// rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1) 
// b�d� poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok�ta o d�ugo�ci bok�w l i k
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

void newGame(double &time, mario_t &mario)
{
	time = 0;
	mario.pos.y = SCREEN_HEIGHT - 16 - mario.stand_l.h;
	mario.pos.x = 2;
}

void jump(mario_t &mario)
{
	for (int i = 0; i < 5; i++)
	{
		mario.pos.y -= i;
	}

	/*for (int i = 0; i < 5; i++)
	{
		mario.pos.y += i;
	}*/
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
	SDL_Surface  *blocks_sprite, *mario_sheet;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	element_t blocks;
	mario_t mario;
	int start_jump = 0, end_jump = 0;
	mario.pos.y = SCREEN_HEIGHT - blocks.ground.h - mario.stand_l.h;
	mario.pos.x = 2;
	mario.curr_frame = &mario.stand_r;


	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe�noekranowy
	/*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	                                 &window, &renderer);
	*/rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
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


	// wy��czenie widoczno�ci kursora myszy
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

	blocks_sprite = SDL_LoadBMP("./blocks_sprite.bmp");
	if (blocks_sprite == NULL) {
		printf("SDL_LoadBMP(blocks_sprite.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	mario_sheet = SDL_LoadBMP("./mario_sheet.bmp");
	if (blocks_sprite == NULL) {
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
		// jaki uplyna� od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

		distance += 10 * delta;

		SDL_FillRect(screen, NULL, niebieski);

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


		// tekst informacyjny
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Esc - wyjscie, n - nowa gra");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		//podloze
		int ground_w = SCREEN_WIDTH / blocks.ground.w;
		for (int i = 0; i < ground_w; i++)
		{
			DrawElement(screen, 1 + i*16, SCREEN_HEIGHT-blocks.ground.w, blocks.ground, blocks_sprite);
		}

		//mario
		DrawElement(screen, mario.pos.x, mario.pos.y, *mario.curr_frame, mario_sheet);


		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs�uga zdarze� (o ile jakie� zasz�y)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_n)
					{
						newGame(worldTime, mario);
					}
					else if (event.key.keysym.sym == SDLK_RIGHT)
					{
						mario.pos.x += SPEED;
						mario.curr_frame = &mario.stand_r;
					}
					else if (event.key.keysym.sym == SDLK_LEFT)
					{
						mario.pos.x -= SPEED;
						mario.curr_frame = &mario.stand_l;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						if(start_jump == 0)
							start_jump=1;
					}
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};

		int a = (worldTime * 100) / 1;
		if (start_jump > 0 && start_jump  <= JUMP_HIGH  && !end_jump && a % 15 == 0)
		{
			mario.pos.y--;
			start_jump++;
			if(start_jump == JUMP_HIGH) end_jump = 1;
		}

		if (end_jump && a % 15 == 0)
		{
			start_jump--;
			mario.pos.y++;
			if (start_jump == 1)
			{
				end_jump = 0;
				start_jump = 0;
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