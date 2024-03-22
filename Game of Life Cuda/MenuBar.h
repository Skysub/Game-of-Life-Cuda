#pragma once
#include "FL\Fl_Box.h"
#include "GameModel.h"

class MenuBar : public Fl_Box {
private:
	GameModel *gm;

public:
	MenuBar(int X, int Y, int W, int H, const char* L, GameModel *gm);
	void draw();

};

