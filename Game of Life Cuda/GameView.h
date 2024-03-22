#pragma once
#ifndef GAMEVIEW_H
#define GAMEVIEW_H

enum CellState;
class GameModel;

#ifndef VEC
#define VEC
typedef struct { int x, y; } vec;
#endif

#include <FL\Fl_Widget.H>

class GameView : public Fl_Widget {
private:
	GameModel* gameModel;
	int errorBoxHeightSum = 0;
	void backupDraw();
	void DrawErrorBox(const char* errorString);
	int handleKB(int key);
	bool mouseToDead;
	vec lastMouseDrag;
	
	int drawCells();
	int drawPixelCells();
	Fl_Color decideColor(CellState state);
	vec screenToCell(vec pix);

public:
	GameView(int X, int Y, int W, int H, const char* L, GameModel *gameModel);
	void draw();
	int handle(int event);

};

#endif