#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}
// okno konsoli nie jest widoczne, jeøeli chcemy zobaczyÊ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniÊ na "Console"
	// console window is not visible, to see the printf output
	//printf("wyjscie printfa trafia do tego okienka\n");
	//printf("printf output goes here\n");

#define SCREEN_WIDTH 850 //px
#define SCREEN_HEIGHT 650 //px
#define MENU_WIDTH 840 //px
#define MENU_HEIGHT 62 //px

#define TLO_KOLOR SDL_MapRGB(gra->screen->format, 0x1A, 0x23, 0x79) //Granatowy
#define CZERWONY SDL_MapRGB(gra->screen->format, 0xFF, 0x00, 0x00)  //Czerwony
#define CZARNY SDL_MapRGB(gra->screen->format, 0x00, 0x00, 0x00)	//Czarny

#define POZIOM_NULL 600 //px
#define POZIOM_ONE 450 //px
#define POZIOM_TWO 300 //px
#define POZIOM_THREE 150 //px
#define LICZBA_POZIOMOW 4 

#define DRABINA_DRAW 95 //px
#define LICZBA_DRABIN_POZIOM 2
#define LICZBA_PLATFORM 3

#define PREDKOSC 100 //prdekosc
#define WYSOKOSC_BITMAP 40 //px

#define ILOSC_DOD_PUNKTOW 5 //na poziom
#define ILOSC_ETAPOW 3

#define DOD_PUNKTY 100 //punkty
#define PUNKTY_ETAP 500 // punkty

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) 
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) 
	{
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
// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawPicture(SDL_Surface *screen, SDL_Surface *picture, int x, int y) 
{
	SDL_Rect rectangle;
	rectangle.x = x - picture->w / 2;
	rectangle.y = y - picture->h / 2;
	rectangle.w = picture->w;
	rectangle.h = picture->h;
	SDL_BlitSurface(picture, NULL, screen, &rectangle);
};
// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface *screen, int x, int y, Uint32 color) 
{
	int bpp = screen->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;
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
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

typedef struct gra 
{
	SDL_Event event;
	SDL_Surface* screen, *charset;
	SDL_Surface* ludzik;
	SDL_Surface* the_end;
	SDL_Surface* koniec;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int mapa = 0;

}Donkey_kong;

typedef struct drabina
{
	SDL_Surface* drabina;
	int x[LICZBA_POZIOMOW][LICZBA_DRABIN_POZIOM];
}drabiny;

typedef struct platforma
{
	SDL_Surface* platforma;
	int y[LICZBA_POZIOMOW];
	int x[LICZBA_POZIOMOW][LICZBA_PLATFORM]; //aby znac lokalizacje kazdej platformy
}platformy;

typedef struct position
{
	double x = 100;
	double y = POZIOM_NULL - 40;
	int poziom = 0;
}Hero;

typedef struct punkty
{
	SDL_Surface* dod_punkty;
	int liczba_punktow = 0;
	int y[ILOSC_ETAPOW][ILOSC_DOD_PUNKTOW];
	int x[ILOSC_ETAPOW][ILOSC_DOD_PUNKTOW];
}punktacja;

typedef struct klawisze
{
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;
}klawisz;


// Wcztywanie i wartosci poczatkowe
int wartosci_poczatkowe(struct gra* gra)
{
	int ekran;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	ekran = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gra->window, &gra->renderer);
	if (ekran != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(gra->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(gra->renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(gra->window, "Donkey_Kong 197912 Karol Banach");

	gra->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	gra->scrtex = SDL_CreateTexture(gra->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	return 1;
}

int wczytaj_bitmapy(struct gra* Gra, struct platforma* platformy, struct drabina* drabiny, struct punkty* punkty)
{
	Gra->charset = SDL_LoadBMP("./cs8x8.bmp");
	platformy->platforma = SDL_LoadBMP("./platforma.bmp");
	drabiny->drabina = SDL_LoadBMP("./drabina.bmp");
	Gra->ludzik = SDL_LoadBMP("./ludzik.bmp");
	Gra->koniec = SDL_LoadBMP("./koniec.bmp");
	Gra->the_end = SDL_LoadBMP("./the_end.bmp");
	punkty->dod_punkty = SDL_LoadBMP("./punkty.bmp");

	if (Gra->charset == NULL || platformy->platforma == NULL || drabiny->drabina == NULL || Gra->ludzik == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		SDL_FreeSurface(Gra->screen);
		SDL_DestroyTexture(Gra->scrtex);
		SDL_DestroyWindow(Gra->window);
		SDL_DestroyRenderer(Gra->renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(Gra->charset, true, 0x000000);

}

void wartosci_platform(struct platforma* platformy)
{
	int l = POZIOM_NULL;
	for (int i = 0; i < LICZBA_POZIOMOW; i++)
	{
		platformy->y[i] = l;
		l = l - 150;
	}
}
void wartosci_drabin_lv1(struct drabina* drabiny)
{
	drabiny->x[0][0] = 335;
	drabiny->x[0][1] = 440;

	drabiny->x[1][0] = 230;
	drabiny->x[1][1] = 545;

	drabiny->x[2][0] = 365;
	drabiny->x[2][1] = 505;
}
void wartosci_platform_lv1(struct platforma* platformy)
{
	platformy->x[0][0] = 150;
	platformy->x[0][1] = 425;
	platformy->x[0][2] = 700;
	platformy->x[1][0] = 182;
	platformy->x[1][1] = 598;
	platformy->x[1][2] = 598;
	platformy->x[2][0] = 388;
	platformy->x[2][1] = 388;
	platformy->x[2][2] = 388;
	platformy->x[3][0] = 210;
	platformy->x[3][1] = 660;
	platformy->x[3][2] = 660;

	platformy->y[0] = POZIOM_NULL;
	platformy->y[1] = POZIOM_ONE;
	platformy->y[2] = POZIOM_TWO;
	platformy->y[3] = POZIOM_THREE;
}
void wartosci_drabin_lv2(struct drabina* drabiny)
{
	drabiny->x[0][0] = 700;
	drabiny->x[0][1] = 740;

	drabiny->x[1][0] = 300;
	drabiny->x[1][1] = 100;

	drabiny->x[2][0] = 400;
	drabiny->x[2][1] = 150;
}
void wartosci_platform_lv2(struct platforma* platformy)
{
	platformy->x[0][0] = 150;
	platformy->x[0][1] = 425;
	platformy->x[0][2] = 700;
	platformy->x[1][0] = 182;
	platformy->x[1][1] = 457;
	platformy->x[1][2] = 700;
	platformy->x[2][0] = 388;
	platformy->x[2][1] = 150;
	platformy->x[2][2] = 388;
	platformy->x[3][0] = 210;
	platformy->x[3][1] = 345;
	platformy->x[3][2] = 345;

	platformy->y[0] = POZIOM_NULL;
	platformy->y[1] = POZIOM_ONE;
	platformy->y[2] = POZIOM_TWO;
	platformy->y[3] = POZIOM_THREE;
}
void wartosci_drabin_lv3(struct drabina* drabiny)
{
	drabiny->x[0][0] = 600;
	drabiny->x[0][1] = 640;

	drabiny->x[1][0] = 200;
	drabiny->x[1][1] = 400;

	drabiny->x[2][0] = 200;
	drabiny->x[2][1] = 505;
}
void wartosci_platform_lv3(struct platforma* platformy)
{
	platformy->x[0][0] = 150;
	platformy->x[0][1] = 425;
	platformy->x[0][2] = 700;
	platformy->x[1][0] = 100;
	platformy->x[1][1] = 598;
	platformy->x[1][2] = 458;
	platformy->x[2][0] = 388;
	platformy->x[2][1] = 388;
	platformy->x[2][2] = 388;
	platformy->x[3][0] = 210;
	platformy->x[3][1] = 660;
	platformy->x[3][2] = 660;

	platformy->y[0] = POZIOM_NULL;
	platformy->y[1] = POZIOM_ONE;
	platformy->y[2] = POZIOM_TWO;
	platformy->y[3] = POZIOM_THREE;
}
void wartosci_punktow(struct punkty* punkty)
{
	//etap 1
	punkty->x[0][0] = 200;
	punkty->y[0][0] = POZIOM_NULL - 40;
	punkty->x[0][1] = 600;
	punkty->y[0][1] = POZIOM_THREE - 40;
	punkty->x[0][2] = 820;
	punkty->y[0][2] = POZIOM_TWO - 40;
	punkty->x[0][3] = 400;
	punkty->y[0][3] = POZIOM_TWO - 40;
	punkty->x[0][4] = 500;
	punkty->y[0][4] = POZIOM_ONE - 85;
	//etap 2
	punkty->x[1][0] = 200;
	punkty->y[1][0] = POZIOM_NULL - 40;
	punkty->x[1][1] = 400;
	punkty->y[1][1] = POZIOM_TWO - 40;
	punkty->x[1][2] = 500;
	punkty->y[1][2] = POZIOM_ONE - 85;
	//etap 3
	punkty->x[2][0] = 50;
	punkty->y[2][0] = POZIOM_ONE - 40;
	punkty->x[2][1] = 470;
	punkty->y[2][1] = POZIOM_TWO - 40;
	punkty->x[2][2] = 800;
	punkty->y[2][2] = POZIOM_THREE - 65;
}

//MENU, INFORMACJE, CZAS
void Draw_menu(struct gra* gra,double *fps,double* worldTime, struct punkty* punktacja)
{
	char text[128];
	DrawRectangle(gra->screen, 4, 4, MENU_WIDTH, MENU_HEIGHT, CZERWONY, CZARNY);
	sprintf(text, "<___Donkey Kong___>");
	DrawString(gra->screen, gra->screen->w / 2 - strlen(text) * 8 / 2, 10, text, gra->charset);
	sprintf(text,"Czas trwania = % .1lf s % .0lf klatek / s  Punkty = %d", *worldTime, *fps,punktacja->liczba_punktow);
	DrawString(gra->screen, gra->screen->w / 2 - strlen(text) * 8 / 2, 24, text, gra->charset);
	sprintf(text, "Esc - wyjscie, n - Nowa Gra, \030 - gora, \031 - dol, \032 - lewo, \033 - prawo, 1,2,3 - dany etap ");
	DrawString(gra->screen, gra->screen->w / 2 - strlen(text) * 8 / 2, 38, text, gra->charset);
	sprintf(text, "Zaimplementowano: A,B,F");
	DrawString(gra->screen, gra->screen->w / 2 - strlen(text) * 8 / 2, 52, text, gra->charset);
}

//WYRYSOWANIE DANEJ MAPY
int what_level(struct position* position)
{
	if (position->y <= POZIOM_NULL - 39 && position->y >= POZIOM_NULL - 41)
		return 0;
	else if (position->y <= POZIOM_ONE - 39 && position->y >= POZIOM_ONE - 41)
		return 1;
	else if (position->y <= POZIOM_TWO - 39 && position->y >= POZIOM_TWO - 41)
		return 2;
	else if (position->y <= POZIOM_THREE - 39 && position->y >= POZIOM_THREE - 41)
		return 3;
	return position->poziom;
}
void ktora_mapa(struct gra* gra, struct platforma* platformy,struct drabina* drabiny,struct punkty* punkty, double* delta)
{
	if (gra->mapa == 0)
	{
		wartosci_platform_lv1(platformy);
		wartosci_drabin_lv1(drabiny);
	}
	if (gra->mapa == 1)
	{
		wartosci_platform_lv2(platformy);
		wartosci_drabin_lv2(drabiny);
	}
	if (gra->mapa == 2)
	{
		wartosci_platform_lv3(platformy);
		wartosci_drabin_lv3(drabiny);;
	}
	if (gra->mapa > 2)
	{
		DrawPicture(gra->screen, gra->the_end,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
		*delta = 0;
	}
}
void zmien_mape(struct gra* gra, struct position* position, struct punkty* punkty)
{
	if (position->x <= 220 && position->x >= 180 && what_level(position) == 3)
	{
		gra->mapa += 1; // zmiana mapy
		punkty->liczba_punktow += PUNKTY_ETAP; //dodanie punktow za etap
		position->x = 100; //px
		position->y = POZIOM_NULL - 40; //px
	}
}
void level(struct gra* gra, struct platforma* platformy, struct drabina* drabiny, struct punkty* punkty,double *delta)
{
	SDL_FillRect(gra->screen, NULL, TLO_KOLOR); //tlo poziomu 

	for (int i = 0; i < LICZBA_POZIOMOW - 1; i++)
	{
		for (int j = 0; j < LICZBA_DRABIN_POZIOM; j++)
		{
			DrawPicture(gra->screen, drabiny->drabina, drabiny->x[i][j], platformy->y[i] - DRABINA_DRAW);
		}
	}
	DrawPicture(gra->screen, gra->koniec,200, POZIOM_THREE - 40);
	for (int i = 0; i < LICZBA_POZIOMOW; i++)
	{
		for (int j = 0; j < LICZBA_PLATFORM; j++)
		{
			DrawPicture(gra->screen, platformy->platforma, platformy->x[i][j], platformy->y[i]);
		}
	}
	for (int j = 0; j < ILOSC_DOD_PUNKTOW; j++)
	{
		DrawPicture(gra->screen, punkty->dod_punkty, punkty->x[gra->mapa][j], punkty->y[gra->mapa][j]);
	}
	
	ktora_mapa(gra, platformy, drabiny,punkty,delta);
}

//PUNKTY
void sprawdz_dod_punkty(struct gra* gra, struct position* position, struct punkty* punkty)
{
	for (int j = 0; j < ILOSC_DOD_PUNKTOW; j++)
	{
		if (position->x <= punkty->x[gra->mapa][j] + 10 && position->x >= punkty->x[gra->mapa][j] - 10 && position->y <= punkty->y[gra->mapa][j]+2 && position->y >= punkty->y[gra->mapa][j] - 2)
		{
			punkty->x[gra->mapa][j] = -10;
			punkty->y[gra->mapa][j] = -10;
			punkty->liczba_punktow += DOD_PUNKTY;
		}
	}
}

//SPACJA I OPADANIE
void opadaj(struct position* position)
{
	position->y = position->y + 1;
}
void sprawdz_podloze(struct position* position, struct platforma* platformy, struct drabina* drabiny)
{
	bool podloze = false;
	position->poziom = what_level(position);
	for (int i = 0; i < LICZBA_PLATFORM; i++)
	{
		if (position->x >= platformy->x[position->poziom][i] - 163 && position->x <= platformy->x[position->poziom][i] + 163 && position->y <= platformy->y[position->poziom]-39 && position->y >= platformy->y[position->poziom] - 41)
			podloze = true;
	}
	for (int i = 0; i < LICZBA_DRABIN_POZIOM; i++)
	{
		if (position->x <= drabiny->x[position->poziom][i] + 1 && position->x >= drabiny->x[position->poziom][i] - 1)
		{
			podloze = true;
		}
		if (position->x <= drabiny->x[position->poziom - 1][i] + 1 && position->x >= drabiny->x[position->poziom - 1][i] - 1)
		{
			podloze = true;
		}
	}
	
	if (podloze == false)
	{
		opadaj(position);
	}
}
void spacja_skok(struct position* position, struct platforma* platformy)
{
	for (int i=0;i<60;i++)
		position->y -= 1;
}

//MECHANIKA RUCHU
void ruch_gora(struct gra* gra, struct position* position, struct platforma* platformy, struct drabina* drabiny, double* delta)
{
	position->poziom = what_level(position);
	if (position->y >= MENU_HEIGHT)
	{
		for (int i = 0; i < LICZBA_DRABIN_POZIOM; i++)
		{
			if (position->x <= drabiny->x[position->poziom][i]+5 && position->x >= drabiny->x[position->poziom][i] - 5)
			{
				position->y = position->y - PREDKOSC * (*delta);
			}
		}
	}
}
void ruch_dol(struct gra* gra, struct position* position, struct platforma* platformy, struct drabina* drabiny, double* delta)
{
	position->poziom = what_level(position);
	if (position->y <= SCREEN_HEIGHT - gra->ludzik->h)
	{
		for (int i = 0; i < LICZBA_DRABIN_POZIOM; i++)
		{
			if (position->x <= drabiny->x[position->poziom-1][i] + 5 && position->x >= drabiny->x[position->poziom-1][i] - 5)
			{
				position->y = position->y + PREDKOSC * (*delta);
			}
		}
	}
}
void ruch_lewo(struct gra* gra, struct position* position, struct platforma* platformy, double *delta)
{
	if (position->x >=  gra->ludzik->w / 2)
	{
		if (position->y+40 <= platformy->y[position->poziom]+1 && position->y + 40 >= platformy->y[position->poziom] - 1)
			position->x = position->x - PREDKOSC* (*delta);
	}
		
}
void ruch_prawo(struct gra* gra, struct position* position,struct platforma* platformy,double* delta)
{
	if (position->x <= SCREEN_WIDTH - gra->ludzik->w/2)
	{
		if (position->y + 40 <= platformy->y[position->poziom]+1 && position->y + 40 >= platformy->y[position->poziom] - 1)
			position->x = position->x + PREDKOSC * (*delta);
	}
}
void ruch(struct klawisze* klawisz, struct gra* gra, struct position* position, struct platforma* platformy, struct drabina* drabiny, double* delta)
{
	if (klawisz->up == true)
	{
		ruch_gora(gra,position,platformy,drabiny, delta);
	}
	if (klawisz->down == true)
	{
		ruch_dol(gra, position, platformy, drabiny,delta);
	}
	if (klawisz->left == true)
	{
		ruch_lewo(gra, position, platformy, delta);
	}
	if (klawisz->right == true)
	{
		ruch_prawo(gra, position, platformy,delta);
	}
}
//ZDARZENIA
int zdarzenia(struct gra* gra, struct position* position, struct drabina* drabiny,struct platforma* platformy,int* quit, double* worldTime, struct klawisze* klawisz,struct punkty* punkty)
{
	while (SDL_PollEvent(&gra->event)) {
		switch (gra->event.type) {
		case SDL_KEYDOWN:
			if (gra->event.key.keysym.sym == SDLK_ESCAPE)
				*quit = 1;
			if (gra->event.key.keysym.sym == SDLK_UP)
			{
				klawisz->up = true;
			}	
			if (gra->event.key.keysym.sym == SDLK_DOWN)
			{
				klawisz->down = true;
			}
			if (gra->event.key.keysym.sym == SDLK_LEFT)
			{
				klawisz->left = true;
			}
			if (gra->event.key.keysym.sym == SDLK_RIGHT)
			{
				klawisz->right = true;
			}
			if (gra->event.key.keysym.sym == SDLK_SPACE)
			{
				spacja_skok(position, platformy);
			}
			if (gra->event.key.keysym.sym == SDLK_n || gra->event.key.keysym.sym == SDLK_1) // nowa gra
			{
				gra->mapa = 0;
				punkty->liczba_punktow = 0;
				*worldTime = 0;
				position->x = 100;
				position->y = POZIOM_NULL - 40;
				position->poziom = 0;
				wartosci_punktow(punkty);
			}
			if (gra->event.key.keysym.sym == SDLK_2)
			{
				gra->mapa = 1;
				position->x = 100;
				position->y = POZIOM_NULL - 40;
				position->poziom = 0;
				wartosci_punktow(punkty);
			}
			if (gra->event.key.keysym.sym == SDLK_3)
			{
				gra->mapa = 2;
				position->x = 100;
				position->y = POZIOM_NULL - 40;
				position->poziom = 0;
				wartosci_punktow(punkty);
			}
			break;
		case SDL_KEYUP:
			if (gra->event.key.keysym.sym == SDLK_UP)
				klawisz->up = false;
			if (gra->event.key.keysym.sym == SDLK_DOWN)
				klawisz->down = false;
			if (gra->event.key.keysym.sym == SDLK_LEFT)
				klawisz->left = false;
			if (gra->event.key.keysym.sym == SDLK_RIGHT)
				klawisz->right = false;
			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		};
	};
	return 0;
}

void generuj(struct gra* gra)
{
	SDL_UpdateTexture(gra->scrtex, NULL, gra->screen->pixels, gra->screen->pitch);
	//SDL_RenderClear(renderer);
	SDL_RenderCopy(gra->renderer, gra->scrtex, NULL, NULL);
	SDL_RenderPresent(gra->renderer);
}
void wyjscie(struct gra* gra)
{
	SDL_FreeSurface(gra->charset);
	SDL_FreeSurface(gra->screen);
	SDL_DestroyTexture(gra->scrtex);
	SDL_DestroyRenderer(gra->renderer);
	SDL_DestroyWindow(gra->window);
	SDL_Quit();
}

// main
#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char **argv) 
{
	Donkey_kong gra;
	platformy platformy;
	drabiny drabiny;
	Hero position;
	klawisze klawisz;
	punktacja punkty;

	int t1 = SDL_GetTicks(), t2, quit=0, frames=0;
	double delta, worldTime=0, fpsTimer=0, fps=0;

	wartosci_poczatkowe(&gra);
	wartosci_platform(&platformy);
	wczytaj_bitmapy(&gra, &platformy,&drabiny,&punkty);
	wartosci_drabin_lv1(&drabiny);
	wartosci_platform_lv1(&platformy);
	wartosci_punktow(&punkty);

	while(!quit) 
	{
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		

		level(&gra,&platformy,&drabiny,&punkty,&delta);
		worldTime += delta;
		Draw_menu(&gra, &fps, &worldTime,&punkty);
		DrawPicture(gra.screen, gra.ludzik, position.x, position.y);
		
		fpsTimer += delta;
		if(fpsTimer > 0.5) 
		{
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
		
		generuj(&gra);
		zdarzenia(&gra,&position,&drabiny,&platformy,&quit,&worldTime,&klawisz,&punkty);
		ruch(&klawisz,&gra,&position,&platformy,&drabiny, &delta);
		sprawdz_podloze(&position, &platformy, &drabiny);
		zmien_mape(&gra, &position, &punkty);
		sprawdz_dod_punkty(&gra, &position, &punkty);
		frames++;
	};
	wyjscie(&gra);
	return 0;
};