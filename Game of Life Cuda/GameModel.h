#pragma once
#ifndef	GAMEMODEL_H
#define GAMEMODEL_H

#define CELLBLOCKSIZEX 16
#define CELLBLOCKSIZEY 16

#define ANIMATEDEAD 3
#define ISOLATION 2
#define SMOTHERING 3

#define DEFAULT_CELL_SIZE 48
#define MAX_ZOOM 4
#define MIN_ZOOM 0.0625f

class GameView;

//Cell state enum
enum CellState {
	DEAD,
	ALIVE, 
	OUT_OF_BOUNDS
};

#ifndef VEC
#define VEC
typedef struct { int x, y; } vec;
#endif
typedef struct { double x, y; } fvec;

void gameCallback(void* in);

class GameModel {
private:
	GameView *gameView;
	vec size, mouseOverCell;
	double zoom;
	fvec pan;
	CellState* cellGrid;
	float simSpeed; //in steps pr. second
	bool running, drawing, hwAccelerated, pixelDraw;
	int pixelZoom;


public:
	GameModel(int sizeX, int sizeY);
	~GameModel();
	void setGameView(GameView* gv);
	int StepSimulation(int n);
	bool stepping;
	CellState getCell(int x, int y);
	void setCell(int x, int y, CellState state);
	void setViewPan(float x, float y);
	void toggleRunning();
	void setSimSpeed(float s);
	void addSimSpeed(float s);
	void addViewPan(int x, int y);
	void addZoom(double z);
	void toggleAcceleration();
	void togglePixelZoom();
	void addPixelZoom(int z);

	fvec getViewPan() const { return pan; }
	vec getMouseOver() const { return mouseOverCell; }
	vec getSize() const { return size; }
	double getZoom() const { return zoom; }
	CellState* getCellGrid() const { return cellGrid; }
	float getSimSpeed() const { return simSpeed; }
	float* getSimSpeedP() { return (&simSpeed); }
	bool getPixelDraw() const { return pixelDraw;  }
	bool getHWAcceleration() const { return hwAccelerated; }
	int getPixelZoom() const { return pixelZoom; }
};

#endif
