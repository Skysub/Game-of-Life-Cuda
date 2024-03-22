
#include <cstdlib>
#include "CudaSimHandler.cuh"
#include <iostream>
#include <FL/Fl.H>
#include "GameModel.h"
#include "GameView.h"
#include "SoftwareSimHandler.h"


//This file's responsebilities include managing the data portion of the game of life simulation

typedef struct { float* speed; GameModel* gm; } CBPackage;

GameModel::GameModel(int sizeX, int sizeY){
	this->size = vec{ sizeX, sizeY };
	cellGrid = (CellState*) malloc(sizeof(CellState) * sizeX * sizeY); //allocating the grid
	if (cellGrid == NULL) {
		std::cerr << "Error when allocating space for the cell grid \n";
		return;
	}

	//Setting the grid whole grid to dead
	for (size_t i = 0; i < sizeX * sizeY; i++){
		cellGrid[i] = DEAD;
	}


	zoom = 1;
	simSpeed = 1;
	stepping = false;
	mouseOverCell = vec{ -1, -1 };
	running = true;
	drawing = false;
	hwAccelerated = true;
	pixelDraw = false;
	pixelZoom = 20;
	pan = fvec{ sizeX / 2.0 * DEFAULT_CELL_SIZE, sizeY / 2.0 * DEFAULT_CELL_SIZE };


	//Setting a deafult staring pattern
	setCell(sizeX / 2 + 5, sizeY / 2 + 3, ALIVE);
	setCell(sizeX / 2 + 6, sizeY / 2 + 4, ALIVE);
	setCell(sizeX / 2 + 4, sizeY / 2 + 5, ALIVE);
	setCell(sizeX / 2 + 5, sizeY / 2 + 5, ALIVE);
	setCell(sizeX / 2 + 6, sizeY / 2 + 5, ALIVE);

	CBPackage *cbp = new CBPackage{ &simSpeed, this};
	Fl::add_timeout(1, gameCallback, cbp);
}

void GameModel::setGameView(GameView* gv) {
	gameView = gv;
}

void gameCallback(void* in) {
	CBPackage cbp = *(reinterpret_cast<CBPackage*>(in));
	//step the simulation
	if (!cbp.gm->stepping) cbp.gm->StepSimulation(-1);

	Fl::repeat_timeout(1.0/(*cbp.speed), gameCallback, in);
}

int GameModel::StepSimulation(int n) {
	if (n == -1) { 
		if(!running) return 0;
		n = 1;
	}

	stepping = true;
	int out;
	if(hwAccelerated) out = SimStep(cellGrid, size, n);
	else out = SoftwareSimStep(&cellGrid, size, n);
	stepping = false;
	if (!drawing) {
		drawing = true;
		gameView->redraw();
		drawing = false;
	}

	return out;
}

GameModel::~GameModel() {
	free(cellGrid);
}

CellState GameModel::getCell(int x, int y){
	if (y < 0 || y > size.y-1) return OUT_OF_BOUNDS;
	if (x < 0 || x > size.x-1) return OUT_OF_BOUNDS;
	int i = x * size.x + y;
	return cellGrid[i];
}

void GameModel::setCell(int x, int y, CellState state) {
	int i = x * size.x + y;
	if (i > size.x * size.y || i < 0) return;
	cellGrid[i] = state;
}

void GameModel::setViewPan(float x, float y) {
	pan.x = x;
	pan.y = y;
}

void GameModel::toggleRunning(){
	running = !running;
}

void GameModel::togglePixelZoom() {
	pixelDraw = !pixelDraw;
}

void GameModel::toggleAcceleration() {
	hwAccelerated = !hwAccelerated;
	//std::cout << "Toggling hardware acceleration to " << hwAccelerated;
}

void GameModel::setSimSpeed(float s){
	if (s < 1) s = 1;
	simSpeed = s;
}

void GameModel::addSimSpeed(float s){
	simSpeed += s;
	if (simSpeed < 1) simSpeed = 1;

}

void GameModel::addViewPan(int x, int y){
	if (pixelDraw) {
		pan.x += x*5;
		pan.y += y*5;
	}
	else {
		pan.x += x;
		pan.y += y;
	}
}

void GameModel::addPixelZoom(int z){
	pixelZoom += z;
	if (pixelZoom < 1) pixelZoom = 1;
}

void GameModel::addZoom(double z){
	double oldZoom = zoom;
	zoom *= z;
	if (zoom < MIN_ZOOM) { 
		zoom = MIN_ZOOM;
		z = zoom / oldZoom;
	}

	if (zoom > MAX_ZOOM) { 
		zoom = MAX_ZOOM;
		z = zoom / oldZoom;
	}

	pan.x += gameView->w()/2;
	pan.y += gameView->h()/2;

	pan.x *= z;
	pan.y *= z;

	pan.x -= gameView->w()/2;
	pan.y -= gameView->h()/2;
}

