#include "stdafx.h"
#include "fishPool.h"
#include <cstdlib>



int fishPool::checkFish(int px, int py){
	if (px >= 0 && px < x && py >= 0 && py < y && pool[getTileByXY(px, py)]->isFish){
		return 1;
	}
	return 0;
}


int fishPool::checkNotFish(int px, int py){
	if (px >= 0 && px < x && py >= 0 && py < y && !pool[getTileByXY(px, py)]->isFish){
		return 1;
	}
	return 0;
}


void fishPool::initMap(const int px, const int py){
	for (int i = 0; i < allFishNum; ++i){
		int rx = rand() % x;
		int ry = rand() % y;
		if (!pool[getTileByXY(rx, ry)]->isFish && rx != px && ry != py){
			pool[getTileByXY(rx, ry)]->isFish = true;
		}
		else{
			--i;
		}
	}

	for (int iy = 0; iy < y; ++iy){
		for (int ix = 0; ix < x; ++ix){
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix - 1, iy - 1);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix - 1, iy);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix - 1, iy + 1);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix, iy - 1);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix, iy + 1);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix + 1, iy - 1);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix + 1, iy);
			pool[getTileByXY(ix, iy)]->fishAround += checkFish(ix + 1, iy + 1);
		}
	}
};


bool fishPool::press(const int px, const int py){
	if (isFirstPress){
		initMap(px, py);
		isFirstPress = false;
	}
	if (pool[getTileByXY(px, py)]->isFish){
		for (auto item : pool){
			if (item->state == flag){
				if (!item->isFish) item->state = mistake;
				continue;
			}
			item->state = normal;
		}
		return false;
	}

	expand(px, py);

	return true;
};


void fishPool::expand(const int px, const int py){
	vector<int> stack;
	if (pool[getTileByXY(px, py)]->state == cover){
		pool[getTileByXY(px, py)]->state = normal;
		++uncoveredNum;
	}
	if (pool[getTileByXY(px, py)]->fishAround == 0)
		stack.push_back(getTileByXY(px, py));
	int ix, iy;
	int pos[3] = { -1, 0, 1 };
	while (!stack.empty()){
		getXYByTile(stack.back(), ix, iy);
		stack.pop_back();

		for (int offset_y : pos){
			for (int offset_x : pos){
				if (checkNotFish(ix + offset_x, iy + offset_y) && pool[getTileByXY(ix + offset_x, iy + offset_y)]->state == cover){
					pool[getTileByXY(ix + offset_x, iy + offset_y)]->state = normal;
					++uncoveredNum;
					if (pool[getTileByXY(ix + offset_x, iy + offset_y)]->fishAround == 0)
						stack.push_back(getTileByXY(ix + offset_x, iy + offset_y));
				}
			}
		}
	}
};


bool fishPool::mark(const int px, const int py){
	if (pool[getTileByXY(px, py)]->state == cover){
		pool[getTileByXY(px, py)]->state = flag;
		++flaged;
		if (pool[getTileByXY(px, py)]->isFish)
			++score;
		else
			--score;
	}
	else if (pool[getTileByXY(px, py)]->state == flag){
		pool[getTileByXY(px, py)]->state = cover;
		--flaged;
		if (pool[getTileByXY(px, py)]->isFish)
			--score;
		else
			++score;
	}

	if (score == allFishNum) return true;

	return false;
};


void fishPool::dbgPrint() const{
	system("cls");
	for (int py = 0; py < y; py++){
		printf("\n");
		for (int px = 0; px < x; px++){
			switch (pool[getTileByXY(px, py)]->state)
			{
			case cover:
				printf("? ");
				break;
			case normal:
				if (pool[getTileByXY(px, py)]->isFish){
					printf("F ");
				}
				else{
					printf("%d ", pool[getTileByXY(px, py)]->fishAround);
				}
				break;
			case flag:
				printf("M ");
				break;
			case mistake:
				printf("X ");
				break;
			}
		}
	}
	printf("\n\n");
};


void fishPool::save(const char *path){
	FILE *fp = fopen(path, "w");
	fprintf(fp, "%d;%d;%d;%d;%d;%d", x, y, allFishNum, score, uncoveredNum, flaged);
	for (auto item : pool){
		fwrite(item, sizeof(*item), 1, fp);
	}
	fclose(fp);
};


void fishPool::load(const char *path){
	FILE *fp = fopen(path, "r");
	int nx, ny;
	fscanf(fp, "%d;%d;%d;%d;%d;%d", &nx, &ny, &allFishNum, &score, &uncoveredNum, &flaged);
	if (x != nx || y != ny){
		return;
	}
	for (auto item : pool){
		fread(item, sizeof(*item), 1, fp);
	}
	isFirstPress = false;
	fclose(fp);
};

