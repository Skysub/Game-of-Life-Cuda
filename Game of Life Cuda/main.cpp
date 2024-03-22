#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>

#include "main.h"
#include "GameModel.h"
#include "CudaSimHandler.cuh"
#include "GameView.h"
#include "MenuBar.h"

const int dSizex = 1280, dSizey = 720;
const int menuBarHeight = 60;
MenuBar* mb;


int main(int argc, char** argv) {

	return CreateView(argc, argv);
}

int CreateView(int argc, char** argv) {
	Fl_Double_Window window(dSizex, dSizey);
	window.box(FL_NO_BOX);
	Fl::visual(FL_RGB);

	GameModel *model = new GameModel(1024, 1024);

	GameView gameWindow(0, 0, dSizex, dSizey - menuBarHeight, "Game View", model);
	model->setGameView(&gameWindow);
	//gameWindow.box(FL_DOWN_BOX);
	gameWindow.color(9);
	gameWindow.labelsize(36);
	gameWindow.align(FL_ALIGN_CLIP);

	//Make the gameWindow the thing that rezises and not the menuBar
	window.resizable(gameWindow);

	MenuBar menuBar(0, dSizey - menuBarHeight, dSizex, menuBarHeight, "", model);
	mb = &menuBar;
	menuBar.box(FL_FLAT_BOX);
	menuBar.color(12);
	menuBar.labelsize(36);
	menuBar.align(FL_ALIGN_CLIP);

	window.end();
	window.show(argc, argv);

	int end = Fl::run();
	int CudaResponse = CudaCleanup(); //Guarantees that we run the function cudaDeviceReset
	if (end == 0) end = CudaResponse;
	return end;
}

void reDrawMenuBar(){
	mb->redraw();
}
