// digFish.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "fishPool.h"

#include <SDL.h>
#include <SDL_framerate.h>
#include <SDL_rotozoom.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

void consoleTest(){
	fishPool pool(10, 5, 10);
	int x, y;
	pool.dbgPrint();
	while (scanf("%d %d", &x, &y)){
		pool.press(x, y);
		pool.dbgPrint();
	}
}

const int SCR_WIDTH = 1500;
const int SCR_HEIGHT = 900;
const int SCR_COLOR_R = 0xff;
const int SCR_COLOR_G = 0xff;
const int SCR_COLOR_B = 0xff;
const int tileSize = 30;
const int seperate = 1;
const int longPressDelay = 200;
int offsetX = 20, offsetY = 15;

const int color_hover = 0xaaaaaa;
const int color_cover = 0xbbbbbb;
const int color_normal = 0xeeeeee;
const int color_mistake = 0xff8fba;

static fishPool pool(40, 28, 200);
SDL_Surface *screen = nullptr;


SDL_Surface *numbers[8];
SDL_Surface *pictures[10];

bool isAlive = true;
bool isWin = false;

int restSaveTimes = 20;
int restLoadTimes = 3;

char buffer[512];


class timer
{
public:
	void startTimer(){ lastTime = clock(); };
	clock_t getTimer(){ return clock() - lastTime; };
private:
	clock_t lastTime;
} myTimer;


struct mouse
{
	bool isPress = false;
	int x = -1, y = -1;
	int hoverTileX = 0, hoverTileY = 0;
} myMouse;


class animateObj
{
private:
	SDL_Rect frame;
	SDL_Rect targetFrame;
	bool active;
public:
	SDL_Surface *pic;
	SDL_Rect* getFrame();
	void setAnimatePosition(int x, int y, int msecond);
	void update(int dt);
};


SDL_Surface *initSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		return nullptr;
	}
	return SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 32, SDL_HWSURFACE);
}


void cls(int x0, int y0, int w, int h, Uint8 r, Uint8 g, Uint8 b){
	Uint32 color;
	color = SDL_MapRGB(screen->format, r, g, b);
	Draw_FillRect(screen, x0, y0, w, h, color);
}


SDL_Surface *preRenderPic(const char *path){
	SDL_Surface *temp = IMG_Load(path);
	SDL_Surface *temp2 = rotozoomSurfaceXY(temp, 0, (double)tileSize / temp->w, (double)tileSize / temp->h, 1);
	SDL_Surface *temp3 = SDL_DisplayFormatAlpha(temp2);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	return temp3;
}


void init_layers(){
	SDL_Color font_colors[8] = {
		{ 231, 122, 5 },
		{ 0.2 * 255, 0.5 * 255, 0 },
		{ 0.4 * 255, 0.8 * 255, 255 },
		{ 0.8 * 255, 0.4 * 255, 255 },
		{ 0.2 * 255, 0, 0.5 * 255 },
		{ 0.5 * 255, 0, 0.5 * 255 },
		{ 255, 0, 0.5 * 255 },
		{ 0.5 * 255, 0, 0.25 * 255 }
	};
	TTF_Font *font = TTF_OpenFont("./ui/consola.ttf", tileSize);
	for (int i = 0; i < 8; ++i){
		sprintf(buffer, "%d", i + 1);
		numbers[i] = TTF_RenderText_Blended(font, buffer, font_colors[i]);
	}
	TTF_CloseFont(font);

	pictures[0] = preRenderPic("./ui/fish.png");
	pictures[1] = preRenderPic("./ui/cat.png");
	pictures[2] = preRenderPic("./ui/cat2.png");
	pictures[3] = preRenderPic("./ui/button.png");
	pictures[4] = preRenderPic("./ui/button1.png");
}


void redraw(int startX, int startY, int endX, int endY){
	for (int y = startY; y < endY; ++y){
		for (int x = startX; x < endX; ++x){
			int drawX = offsetX + (tileSize + seperate) * x;
			int	drawY = offsetY + (tileSize + seperate) * y;

			switch (pool.getState(x, y))
			{
			case cover:
				if (myMouse.hoverTileX == x && myMouse.hoverTileY == y){
					//Draw_FillRect(screen, drawX, drawY, tileSize, tileSize, color_hover);
					SDL_Rect rect = { drawX + (tileSize - pictures[4]->w) / 2, drawY + (tileSize - pictures[4]->h) / 2, tileSize, tileSize };
					SDL_UpperBlit(pictures[4], NULL, screen, &rect);
					
				}
				else{
					//Draw_FillRect(screen, drawX, drawY, tileSize, tileSize, color_cover);
					SDL_Rect rect = { drawX + (tileSize - pictures[3]->w) / 2, drawY + (tileSize - pictures[3]->h) / 2, tileSize, tileSize };
					SDL_UpperBlit(pictures[3], NULL, screen, &rect);
				}
				break;
			case normal:
				Draw_FillRect(screen, drawX, drawY, tileSize, tileSize, color_normal);
				if (pool.getFishState(x, y)){
					SDL_Rect rect = { drawX + (tileSize - pictures[0]->w) / 2, drawY + (tileSize - pictures[0]->h) / 2, tileSize, tileSize };
					SDL_UpperBlit(pictures[0], NULL, screen, &rect);
				}
				else{
					if (pool.getFishAround(x, y) != 0){
						SDL_Rect rect = { drawX + (tileSize - numbers[0]->w) / 2, drawY + (tileSize - numbers[0]->h) / 2, tileSize, tileSize };
						SDL_UpperBlit(numbers[pool.getFishAround(x, y) - 1], NULL, screen, &rect);
					}
				}
				break;
			case flag:
			{
				Draw_FillRect(screen, drawX, drawY, tileSize, tileSize, color_normal);
				SDL_Rect rect = { drawX + (tileSize - pictures[1]->w) / 2, drawY + (tileSize - pictures[1]->h) / 2, tileSize, tileSize };
				SDL_UpperBlit(pictures[1], NULL, screen, &rect);
				break;
			}
			case mistake:
				Draw_FillRect(screen, drawX, drawY, tileSize, tileSize, color_mistake);
				SDL_Rect rect = { drawX + (tileSize - pictures[2]->w) / 2, drawY + (tileSize - pictures[2]->h) / 2, tileSize, tileSize };
				SDL_UpperBlit(pictures[2], NULL, screen, &rect);
				break;
			}
		}
	}
}


void getTile(const int px, const int py, int &x, int &y){
	x = (px - offsetX) / (tileSize + seperate);
	y = (py - offsetY) / (tileSize + seperate);
	if (x >= pool.getX() || y >= pool.getY() || x < 0 || y < 0){
		x = y = -1;
	}
}


inline void checkTimer(){
	if (!myMouse.isPress || !isAlive) return;
	if (myTimer.getTimer() >= longPressDelay){
		int x, y;
		getTile(myMouse.x, myMouse.y, x, y);
		if (isAlive && pool.mark(x, y)){
			isWin = true;
			MessageBoxA(FindWindowA("SDL_app", "SDL_app"), "Win", "", MB_OK | MB_SYSTEMMODAL);
			return;
		}
		myMouse.isPress = false;
		redraw(x, y, x + 1, y + 1);
		SDL_Flip(screen);
		//pool.dbgPrint();
	}
}


inline void printInfo(char *info, TTF_Font *font, SDL_Rect &pos){
	SDL_Surface *infoLayer = TTF_RenderText_Blended(font, buffer, { 0x00, 0x00, 0x00 });
	SDL_UpperBlit(infoLayer, NULL, screen, &pos);
	SDL_FreeSurface(infoLayer);
	SDL_UpdateRect(screen, pos.x, pos.y, pos.w, pos.h);
}


void winAnimate(){
	
}


void loseAnimate(){

}


int _tmain(int argc, _TCHAR* argv[])
{

	ShowWindow(GetConsoleWindow(), 0);

	if ((screen = initSDL()) == 0) {
		return -1;
	}

	SDL_WM_SetCaption("Dig Fish", NULL);

	TTF_Init();

	TTF_Font *font = TTF_OpenFont("./ui/consola.ttf", 16);

	init_layers();

	cls(0, 0, SCR_WIDTH, SCR_HEIGHT, SCR_COLOR_R, SCR_COLOR_G, SCR_COLOR_B);
	redraw(0, 0, pool.getX(), pool.getY());
	SDL_Flip(screen);

	FPSmanager manager;
	SDL_initFramerate(&manager);
	SDL_setFramerate(&manager, 60);

	SDL_Event event;
	Uint8 *keys;

	clock_t startTime = clock();


	bool RUN = true;
	while (RUN) {
		checkTimer();
		while (SDL_PollEvent(&event)) {
			keys = SDL_GetKeyState(NULL);
			switch (event.type) {
			case SDL_KEYDOWN:
				if (keys[SDLK_s]){
					if (!restSaveTimes) break;
					--restSaveTimes;
					pool.save("./saves/fish.txt");
					break;
				}
				if (keys[SDLK_l]) {
					if (!restLoadTimes) break;
					pool.load("./saves/fish.txt");
					isAlive = true;
					isWin = false;
					redraw(0, 0, pool.getX(), pool.getY());
					SDL_Flip(screen);
					--restLoadTimes;
					break;
				}
				if (keys[SDLK_k]) {
					winAnimate();
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				myMouse.isPress = true;
				myMouse.x = event.button.x;
				myMouse.y = event.button.y;
				myTimer.startTimer();
				break;
			case SDL_MOUSEMOTION:
			{
				int x, y;
				getTile(event.button.x, event.button.y, x, y);
				if (myMouse.hoverTileX != -1 && (x != myMouse.hoverTileX || y != myMouse.hoverTileY)){
					int oldX = myMouse.hoverTileX, oldY = myMouse.hoverTileY;
					myMouse.hoverTileX = x;
					myMouse.hoverTileY = y;
					redraw(oldX, oldY, oldX + 1, oldY + 1);
				}
				if (x != -1){
					myMouse.hoverTileX = x;
					myMouse.hoverTileY = y;
					redraw(x, y, x + 1, y + 1);
				}
				SDL_Flip(screen);
				break;
			}
			case SDL_MOUSEBUTTONUP:
				myMouse.isPress = false;
				if (myTimer.getTimer() < longPressDelay){
					int x, y;
					getTile(event.button.x, event.button.y, x, y);
					if (x != -1){
						isAlive = pool.press(x, y);
						if (!isAlive){
							MessageBoxA(FindWindowA("Dig Fish", "Dig Fish"), "lose", "", MB_OK | MB_SYSTEMMODAL);
						}
					}
					else if (!isAlive || isWin){
						isAlive = true;
						startTime = clock();
						pool.reset();
						restLoadTimes = 3; 
						restSaveTimes = 20;
						redraw(0, 0, pool.getX(), pool.getY());
						SDL_Flip(screen);
					}
				}
				redraw(0, 0, pool.getX(), pool.getY());
				SDL_Flip(screen);
				//pool.dbgPrint();
				break;
			case SDL_QUIT:
				RUN = false;
				break;
			}
		}

		SDL_Rect pos = { 1270, 20, 250, 150 };
		SDL_FillRect(screen, &pos, 0xffffff);

		clock_t sPast = (clock() - startTime) / 1000;
		sprintf(buffer, "Time Past:%d:%02d:%02d", sPast / 3600, sPast % 3600 / 60, sPast % 60);
		printInfo(buffer, font, pos);

		pos.y = 40;
		sprintf(buffer, "Fish num:%d", pool.getFishNum());
		printInfo(buffer, font, pos);

		pos.y = 60;
		sprintf(buffer, "Fish pool remain:%d", pool.getTileNum() - pool.getUncoveredNum());
		printInfo(buffer, font, pos);

		pos.y = 80;
		sprintf(buffer, "Marked Fish:%d", pool.getFlagedNum());
		printInfo(buffer, font, pos);

		pos.y = 100;
		sprintf(buffer, "Remain Save Times:%d", restSaveTimes);
		printInfo(buffer, font, pos);

		pos.y = 120;
		sprintf(buffer, "Remain Load Times:%d", restLoadTimes);
		printInfo(buffer, font, pos);

		pos.y = 20;
		SDL_UpdateRect(screen, pos.x, pos.y, pos.w, pos.h);


		SDL_framerateDelay(&manager);
	}

	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}

