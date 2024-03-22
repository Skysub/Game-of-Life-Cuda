#include "MenuBar.h"
#include <FL/fl_draw.H>
#include <string>

MenuBar::MenuBar(int X, int Y, int W, int H, const char* L, GameModel *gameModel = NULL) : Fl_Box(X, Y, W, H, L) {
	this->gm = gameModel;
}

void MenuBar::draw() {
	__super::draw();
	int textHeight = 10;
	int textWidth = 10;
	fl_measure("Text", textWidth, textHeight);
	vec middlePoint{x()+w()/2-40,textHeight/2+y()+h()/2 - 10};
	fl_font(FL_HELVETICA, 24);

	//Draw information here
	std::string s;
	int left = middlePoint.x, right = middlePoint.x;

	//Size of cell grid
	s = "Grid: " + std::to_string(gm->getSize().x) + "x" + std::to_string(gm->getSize().y);
	fl_measure(s.c_str(), textWidth, textHeight);
	left -= textWidth + 100;
	fl_draw(s.c_str(), left, middlePoint.y - 10);

	//Zoom level
	if(gm->getPixelDraw()) s = "Zoom: " + std::to_string(gm->getPixelZoom());
	else s = "Zoom: " + std::to_string(gm->getZoom());

	fl_measure(s.c_str(), textWidth, textHeight);
	//left -= textWidth + 100;
	fl_draw(s.c_str(), left, middlePoint.y + 10);

	//Sim speed
	s = "Simulations per second: " + std::to_string((int)gm->getSimSpeed());

	fl_measure(s.c_str(), textWidth, textHeight);
	fl_draw(s.c_str(), right, middlePoint.y - 10);
	//right += textWidth + 100;

	//Hardware acceleration state
	std::string s2 = "No";
	if (gm->getHWAcceleration()) s2 = "Yes";
	s = "Hardware Acceleration: " + s2;

	fl_measure(s.c_str(), textWidth, textHeight);
	fl_draw(s.c_str(), right, middlePoint.y + 10);
	
	//Tell the user to press h for help
	right += textWidth + 190;

	fl_draw("Press h for controls", right, middlePoint.y + 10);
}