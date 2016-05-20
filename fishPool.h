#include <vector>

using namespace std;

enum tileState
{
	cover,
	normal,
	flag,
	mistake
};

struct tile
{
	//关于单个鱼池的结构
	bool isFish;
	tileState state;
	int fishAround;

	tile(){
		resetTile();
	}

	void resetTile(){
		isFish = false;
		state = cover;
		fishAround = 0;
	}
};

class fishPool
{
private:
	int x, y;
	int flaged;
	int allFishNum;
	int allTileNum;
	int score;
	int uncoveredNum;
	bool isFirstPress;
	vector<tile *> pool;

	void initMap(const int px, const int py);
	int checkFish(const int px, const int py);
	int checkNotFish(const int px, const int py);

public:
	fishPool(int ix, int iy, int fishNum) :x(ix), y(iy), allFishNum(fishNum), flaged(0), allTileNum(ix * iy), uncoveredNum(0), isFirstPress(true)
	{
		srand(clock());
		for (int i = 0; i < allTileNum; ++i){
			pool.push_back(new tile);
		}
	
	};

	~fishPool(){
		for (auto item : pool){
			delete item;
		}
		pool.clear();
	}

	void reset(){
		flaged = 0;
		score = 0;
		uncoveredNum = 0;
		isFirstPress = true;
		for (auto item : pool){
			item->resetTile();
		}
	}

	int getTileByXY(const int px, const int py) const{
		return x * py + px;
	}

	void getXYByTile(const int tile, int &px, int &py) const{
		px = tile % x;
		py = tile / x;
	}

	int getX() const{ return x; };

	int getY() const{ return y; };

	tileState getState(const int px, const int py) const{ return pool[getTileByXY(px, py)]->state; };
	
	bool getFishState(const int px, const int py) const{ return pool[getTileByXY(px, py)]->isFish; };

	int getFishAround(const int px, const int py) const{ return  pool[getTileByXY(px, py)]->fishAround; };

	int getTileNum(){
		return allTileNum;
	}

	int getFishNum(){
		return allFishNum;
	}

	int getUncoveredNum(){
		return uncoveredNum;
	}

	int getFlagedNum(){
		return flaged;
	}

	void expand(const int px, const int py);

	bool press(const int px, const int py);

	bool mark(const int px, const int py);

	void dbgPrint() const;

	void save(const char *path);

	void load(const char *path);

};