// Screen.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <array>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include "Utils.h"
#include <ctime>

using namespace std;

class GameObject {
	bool		enabled;

protected:

	Position	pos;
	Screen&		screen;
	char*		shape;
	int			width;
	int			height;

	vector<GameObject *> children;
	GameObject*	parent;

public:
	GameObject(const char* shape, int width, int height, const Position& pos = Position{ 0, 0 })
		: height(height), width(width), shape(nullptr), pos(pos), enabled(true), parent(nullptr),
		screen(Screen::getInstance()) {
		if (!shape || strlen(shape) == 0)
			this->shape = nullptr;
		else {
			this->shape = new char[width*height];
			strncpy(this->shape, shape, width*height);
		}
		children.clear();
	}

	GameObject(int width, int height, const Position& pos = Position{ 0, 0 })
		: GameObject(nullptr, width, height, pos)
	{}

	virtual ~GameObject()
	{
		while (children.size() > 0) {
			auto back = children.back();
			children.pop_back();
			delete back;
		}
		if (shape) { delete[] shape; }
		shape = nullptr;
		width = 0, height = 0;
	}

	void setParent(GameObject* parent) {
		this->parent = parent;
	}

	void add(GameObject* child) {
		if (!child) return;

		child->setParent(this);
		children.push_back(child);
	}

	virtual bool isActive() { return enabled; }

	void setActive(bool flag = true) { enabled = flag; }

	void setShape(const char* shape) {
		if (!shape) return;
		strncpy(this->shape, shape, width*height);
	}

	virtual void setPos(int x, int y) { this->pos.x = x; this->pos.y = y; }

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	Position& getPos() { return pos; }

	vector<GameObject *>& getChildren() {
		return children;
	}

	void internalDraw(const Position& inheritedPos = Position{ 0, 0 }) {
		if (!enabled) return;

		draw(inheritedPos);

		for (auto& child : children) child->internalDraw(pos + inheritedPos);
	}

	virtual void draw(const Position& inheritedPos = Position{ 0, 0 }) {
		screen.draw(shape, width, height, pos + inheritedPos);
	}

	void internalUpdate() {
		if (!enabled) return;
		update();
		for (auto& child : children) child->internalUpdate();
	}

	virtual void update() {}

	static const char rect = '\xB2';
};


class Panel : public GameObject {

public:
	Panel(const char* layout, int width, int height, const Position& pos) : GameObject(layout, width, height, pos) {}

	GameObject* pop() {
		auto& children = getChildren();
		auto back = children.back();
		children.pop_back();
		back->setParent(nullptr);
		return back;
	}

	void draw(const Position& inheritedPos) {
		screen.drawRect(Position{ pos.x - 1, pos.y - 1 } +inheritedPos, getWidth() + 2, getHeight() + 2);
	}
};

class Text : public Panel {

public:
	Text(const char* info, const Position& pos) : Panel(info, strlen(info), 1, pos) {}

	void draw(const Position& inheritedPos) {
		screen.draw(shape, width, 1, pos + inheritedPos);
	}
};

class Item : public Text {
	int items;
	char* buf;

public:
	Item(const Position& pos, int total = 0) : Text("Remaining Items:     ", pos), items(total) {
		buf = new char[strlen(shape) + 1];
	}

	~Item() {
		delete[] buf;
	}

	//void addScore(int inc) {
	//	if (inc < 0) return;
	//	score += inc;
	//}

	void decTotal(int dec) {
		if (dec < 0) return;
		items -= dec;
	}

	void draw(const Position& inheritedPos) {
		sprintf(buf, "Left Items: %3d", items);
		screen.draw(buf, strlen(buf), 1, pos + inheritedPos);
	}
};

class Map : public Panel {
	bool* map;
	int   width;
	int	  height;
	int   upper;
	char  buffer[100];
	Item* items;
	int numOfItems;

public:
	Map(int width, int height, const Position& pos)
		: map(new bool[width*height]), width(width), height(height), upper(height - 1),
		Panel(nullptr, width, height, pos), items(nullptr), numOfItems(0)
	{
		for (int i = 0; i < width*height; i++)
		{
			map[i] = false;
		}
	}

	~Map() { if (map) delete[] map; }

	//void setScore(Item* score) {
	//	this->score = score;
	//}

	//void addScore(int score) {
	//	if (this->score) this->score->addScore(score);
	//}

	void setItems(int items) {
		numOfItems = items;
	}

	void decTotal(int items) {
		if (this->items) this->items->decTotal(items);
	}

	bool isOpened(Position pos) {
		if (map[pos.x + pos.y*this->width])
			return true;
		else
			return false;
	}

	//bool isLineAllOccupied(int line) {
	//	for (int i = 0; i < width; i++) {
	//		if (!map[line*width + i]) {
	//			return false;
	//		}
	//	}
	//	return true;
	//}

	//bool evaluateLine(int line) {
	//	if (!isLineAllOccupied(line)) return false;

	//	// clean the given line
	//	for (int i = 0; i < width; i++) map[line*width + i] = false;

	//	// copy lines above the "line" down below their below lines.
	//	for (int i = line - 1; i >= (upper-1) && i >= 0; i--) {
	//		for (int j = 0; j < width; j++) 
	//			map[(i + 1)*width + j] = map[i*width + j];
	//		if (i==0) for (int i = 0; i < width; i++) 
	//			map[line*width + i] = false;
	//	}
	//	upper++;
	//	return true;
	//}

	//bool isGrounded(const char* shape, const Position& pos, int w, int h) {
	//	if (pos.y + h >= height) return true;

	//	// pos.y + h < height
	//	int next = pos.y + h;
	//	if (next < upper) return false;
	//	for (int i = 0; i < h; ++i) {
	//		for (int j = 0; j < w; j++)
	//			if (map[pos.x + j + (pos.y + i + 1) * width] && shape[j + i*w] != ' ')
	//				return true;
	//	}
	//	return false;
	//}

	//bool isOccupied(const Position& pos) {
	//	return map[pos.x + pos.y *width];
	//}

	//void place(const char* shape, const Position& pos, int w, int h) {
	//	for (int i = 0; i < h; i++) {
	//		for (int j = 0; j < w; j++) {
	//			if (shape[j + i*w] != ' ')
	//				map[pos.x + j + (pos.y + i)* width] = true;
	//		}
	//	}
	//	if (pos.y < upper) upper = pos.y;
	//	addScore(1);
	//}

	void draw(const Position& inheritedPos)
	{
		screen.drawRect(Position{ pos.x - 1, pos.y - 1 } +inheritedPos, width + 2, height + 2);
		for (int i = upper; i < height; i++) {
			memset(buffer, ' ', 100);
			buffer[width] = '\0';
			for (int j = 0; j < width; j++) {
				if (map[j + i * width]) buffer[j] = GameObject::rect;
			}
			screen.draw(buffer, width, 1, Position{ pos.x, pos.y + i } +inheritedPos);
		}
	}
};

struct MonsterShape {
	string shape;
	int width;
	int height;
};

class Monster : public GameObject {
	float x;
	float y;
	float speed;
	int items;
	Map* map;
	//bool interactable;

public:
	Monster(const MonsterShape& shape, bool interactable = true, const Position& pos = Position{ 0, 0 })
		: GameObject(shape.shape.c_str(), shape.width, shape.height, pos), x(.0f), y(.0f), speed(0.02f), map(nullptr), items(0)
		//, interactable(interactable)
	{ }

	//void rotateShape() {
	//	static char* shape = new char[GameObject::width*GameObject::height];
	//	for (int y = 0; y < height; y++)
	//		for (int x = 0; x < width; x++)
	//			shape[(GameObject::width-1-x)*GameObject::height+y] = this->shape[y*GameObject::width + x];
	//	setShape(shape);
	//	swap(GameObject::width, GameObject::height);
	//}

	//void setInteractable() { interactable = true;  }

	void setPos(int x, int y) {
		GameObject::setPos(x, y);
	}

	void setMap(Map* map) {
		this->map = map;
	}

	void moveAround(Map* map, Position pos) {
		srand(time(NULL));
		Position temp;
		temp.x = pos.x;
		temp.y = pos.y;
		while (true) {
			switch (rand() % 8)
			{
			case 0:
				pos.x--; pos.y--; break;
			case 1:
				pos.y--; break;
			case 2:
				pos.x++; pos.y--; break;
			case 3:
				pos.x--; break;
			case 4:
				pos.x++; break;
			case 5:
				pos.x--; pos.y++; break;
			case 6:
				pos.y++; break;
			case 7:
				pos.x++; pos.y++; break;
			}
			if (pos.x == 0 || pos.y == 0 || pos.x == map->getWidth() + 1 || pos.y == map->getHeight() + 1) {
				pos.x = temp.x;
				pos.y = temp.y;
			}
		}
		return;
	}

	void eat(Position pos) {
		if (!map->isOpened(pos)) {
			items++;
		}
		return;
	}

	void update() {
		if (isActive() == false) return;

		moveAround(map, this->pos);
		eat(this->pos);

		//if (Input::GetKeyDown(KeyCode::Right)) {
		//	x++;
		//	if (parent) {
		//		if (x + width >= parent->getWidth()) x = parent->getWidth() - width;
		//	}
		//	else {
		//		if (x + width >= screen.getWidth()/2) x = screen.getWidth() - width;
		//	}
		//}
		//if (Input::GetKeyDown(KeyCode::Left)) {
		//	x--;
		//	if (x < 0.0f) x = 0.0f;
		//}
		//if (Input::GetKeyDown(KeyCode::Up)) {
		//	rotateShape();
		//}
		//if (Input::GetKeyDown(KeyCode::Down)) {
		//	speed *= 2;
		//}			
		//if (Input::GetKeyDown(KeyCode::Space)) {
		//	if (map) {
		//		pos.y = y;
		//		while (!map->isGrounded(shape, pos, width, height)) {
		//			pos.y++; y+= 1.0f;
		//		}
		//		return;
		//	}
		//}
		//if (Input::GetKeyDown(KeyCode::A)) {
		//	rotateShape();
		//}
		//y += speed;
	}

	void draw(const Position& inheritedPos) {

		//if (interactable == true) {
		//	pos.x = (int)x, pos.y = (int)y;
		//}
		//
		//for (int i = pos.y + 1; map && i < map->getHeight(); i++) {
		//	if ( map->isOccupied(Position{pos.x, i}))
		//		break;
		//	screen.draw("\xFA", 1, 1, Position{ pos.x, i } + inheritedPos);
		//}
		//for (int i = pos.y + 1; map && i < map->getHeight(); i++) {
		//	if (map->isOccupied(Position{ pos.x + width - 1, i }))
		//		break;
		//	screen.draw("\xFA", 1, 1, Position{ pos.x + width - 1, i } +inheritedPos);
		//}
		screen.draw(shape, width, height, pos + inheritedPos);
	}
};

int main()
{
	Screen&	 screen = Screen::getInstance();
	vector<GameObject *> gameObjects;
	vector<GameObject *> monsters;

	//string mode = "mode con cols=" + to_string(screen.getWidth() + 10);
	//mode += " lines=" + to_string(screen.getHeight() + 5);

	//std::system(mode.c_str());
	std::system("chcp 437");

	auto mainMap = new Map(50, 15, Position{ 1, 1 });
	auto& children = mainMap->getChildren();

	int numOfMonsters = monsters.size();
	auto statePanel = new Panel(nullptr, mainMap->getWidth(), 3 + numOfMonsters, Position{ 1, mainMap->getHeight() + 1 });
	statePanel->add(new Text("Total Movement = ", Position{ 1, 1 }));
	auto items = new Item(Position{ 1, 2 }, mainMap->getWidth() * mainMap->getHeight() / 2);
	statePanel->add(items);

	mainMap->setItems(mainMap->getWidth() * mainMap->getHeight());

	gameObjects.push_back(mainMap);
	gameObjects.push_back(statePanel);

	screen.clear(); screen.render();

	while (!Input::GetKeyDown(KeyCode::Esc)) {
		screen.clear();
		for (auto obj : gameObjects) obj->internalUpdate();

		for (auto it = gameObjects.cbegin();
			it != gameObjects.cend(); it++)
			(*it)->internalDraw();

		screen.render();
		Sleep(1000);

		Input::EndOfFrame();
	}

	while (gameObjects.size() > 0) {
		auto back = gameObjects.back();
		gameObjects.pop_back();
		delete back;
	}

	return 0;
}