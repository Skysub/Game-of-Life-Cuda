#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cmath>
#include <FL/names.h>;

#include "GameView.h"
#include "GameModel.h"
#include <array>
#include "main.h"
#include <iostream>

GameView::GameView(int X, int Y, int W, int H, const char* L = 0, GameModel *gameModel = NULL) : Fl_Widget(X, Y, W, H, L) {
    this->gameModel = gameModel;
    mouseToDead = false;
};

void GameView::draw() {
    errorBoxHeightSum = 0;

    fl_color(FL_BLUE);
    fl_rectf(x(), y(), w(), h());

    int drawCheck = 0;
    if (gameModel->getPixelDraw()) drawCheck = drawPixelCells();
    else drawCheck = drawCells();

    if(drawCheck == 1) GameView::backupDraw();
}

int GameView::drawCells() {
    //Determining the cells we can see
    double cellSize = DEFAULT_CELL_SIZE * gameModel->getZoom();
    fvec pan = gameModel->getViewPan();
    vec viewCellPos = vec{ (int)(pan.x / cellSize), (int)(pan.y / cellSize) };
    double hCells = ((double) w() + (int)pan.x % (int)std::ceil(cellSize)) / cellSize;
    double vCells = ((double) h() + (int)pan.y % (int)std::ceil(cellSize)) / cellSize;
    vec viewCellAmount = vec{ 1 + (int) std::ceil(hCells), 1 + (int)std::ceil(vCells) };

    //Draw the cells
    for (int i = 0; i < viewCellAmount.x; i++){
        for (int j = 0; j < viewCellAmount.y; j++){
            CellState state = gameModel->getCell(viewCellPos.x + i, viewCellPos.y + j);
            if (state == OUT_OF_BOUNDS) continue;
            fl_color(decideColor(state));

            //printf("Drawing cell %d,%d", i, j);
            fl_rectf(-(int)pan.x % (int)std::ceil(cellSize) + i * cellSize, -(int)pan.y % (int)std::ceil(cellSize) + j * cellSize, cellSize, cellSize);
        }
    }

    //Don't draw the outlines if the zoom level is very low
    if (gameModel->getZoom() < 0.3) return 0;

    //Draw the cell outlines
    fl_color(50,50,50);
    for (size_t i = 0; i < viewCellAmount.x; i++){
        int x = -(int)pan.x % (int)std::ceil(cellSize) + i * cellSize;
        fl_line(x, 0, x, h());
    }
    for (size_t i = 0; i < viewCellAmount.y; i++) {
        int y = -(int)pan.y % (int)std::ceil(cellSize) + i * cellSize;
        fl_line(0, y, w(), y);
    }    
    return 0;
}

int GameView::drawPixelCells() {
    int cellSize = gameModel->getPixelZoom();
    //bool lines = cellSize > 19;
    fvec pan = gameModel->getViewPan();
    vec size = gameModel->getSize();
    vec topLeftCell = vec{ (int)(pan.x / DEFAULT_CELL_SIZE), (int)(pan.y / DEFAULT_CELL_SIZE) };
    vec offset = vec{(int)pan.x % cellSize, (int)pan.y % cellSize };
    Fl_Color currentColor = FL_BLACK;
    //bool* lineInfo = (bool*) malloc(sizeof(bool) * (w() + h()));
    //if (lineInfo == NULL) return 1;

    //Constructing the lineInfo array if needed
    /*if (lines) {
        for (size_t i = 0; i < w(); i++) {
            if ((i - offset.x) % cellSize == 0) lineInfo[i] = true;
            else lineInfo[i] = false;
        }
        for (size_t i = 0; i < h(); i++) {
            if ((i - offset.y) % cellSize == 0) lineInfo[i + w()] = true;
            else lineInfo[i + w()] = false;
        }
    } else {
        for (size_t i = 0; i < w() + h(); i++) {
            lineInfo[i] = false;
        }
    }*/

    uint32_t *xCellCoord = (uint32_t*)malloc(sizeof(uint32_t) * w());
    uint32_t *yCellCoord = (uint32_t*)malloc(sizeof(uint32_t) * h());
    if (xCellCoord == NULL || yCellCoord == NULL) return 1;

    //Constructing lookup tables for screen space to cell space conversion
    for (size_t i = 0; i < w(); i++){
        xCellCoord[i] = topLeftCell.x + i/cellSize;
    }
    for (size_t i = 0; i < h(); i++){
        yCellCoord[i] = topLeftCell.y + i / cellSize;
    }

    uchar* bitmap = (uchar*) malloc(3 * sizeof(uchar) * (h() * w()));
    if (bitmap == NULL) {
        free(xCellCoord);
        free(yCellCoord);
        return 1; 
    }

    int a = w();
    CellState state;
    for (size_t i = 0; i < w(); i++){
        //if (lineInfo[i]) {
        //    fl_color(50, 50, 50);
        //    for (size_t j = 0; j < h(); j++){
        //        fl_point(i, j);
        //    }
        //} else {
            for (size_t j = 0; j < h(); j++){
                //if (lineInfo[w() + j]) {
                //    fl_color(50, 50, 50);
                //} else {
                state = gameModel->getCell(xCellCoord[i], yCellCoord[j]);

                switch (state){
                    case DEAD:
                        bitmap[3 * (i + w() * j)] = 0;
                        bitmap[3 * (i + w() * j) + 1] = 0;
                        bitmap[3 * (i + w() * j) + 2] = 0;
                        break;

                    case ALIVE:
                        bitmap[3 * (i + w() * j)] = 255;
                        bitmap[3 * (i + w() * j) + 1] = 255;
                        bitmap[3 * (i + w() * j) + 2] = 255;
                        break;

                    case OUT_OF_BOUNDS:
                        bitmap[3 * (i + w() * j)] = 50;
                        bitmap[3 * (i + w() * j) + 1] = 50;
                        bitmap[3 * (i + w() * j) + 2] = 50;
                        break;

                    default:
                        break;
                }
                //}
            }
        //}
    }
    fl_draw_image(bitmap, 0, 0, w(), h());
    free(xCellCoord);
    free(yCellCoord);
    free(bitmap);
    return 0;
}

Fl_Color GameView::decideColor(CellState state) {
    switch (state){
    case DEAD:
        return FL_BLACK;
    case ALIVE:
        return FL_WHITE;
    default:
        return FL_BLUE;
    }
}

void GameView::backupDraw() {
    //Establish a background
    fl_color(FL_GRAY0);
    fl_rectf(x(), y(), w(), h());

    // DRAW BLACK 'X'
    fl_color(FL_BLACK);
    int x1 = x(), y1 = y();
    int x2 = x() + w() - 1, y2 = y() + h() - 1;
    fl_line(x1, y1, x2, y2);
    fl_line(x1, y2, x2, y1);

    DrawErrorBox("Error drawing the image");
}

void GameView::DrawErrorBox(const char* errorString) {
    //Draw error box
    fl_font(FL_COURIER, 12);

    int tw = 0, th = 0;
    fl_measure(errorString, tw, th);

    int x2 = x() + w() - 1, y2 = y() + h() - 1;
    fl_draw_box((FL_BORDER_BOX), x2 / 2 - (tw + 20) / 2, y2 / 2 - (th + 10) / 2 + errorBoxHeightSum, tw + 20, th + 10, FL_WHITE);
    fl_color(FL_BLACK);
    fl_draw(errorString, x2 / 2 - tw / 2, y2 / 2 + th / 2 - 3 + errorBoxHeightSum);
    errorBoxHeightSum += th + 20;
}

int GameView::handle(int event) {
    int out;

    switch (event){
    case FL_KEYDOWN:
        out = handleKB(Fl::event_key());
        reDrawMenuBar();
        return out;

    case FL_PUSH:
        
        lastMouseDrag = vec{ Fl::event_x(), Fl::event_y() };
        vec cell = screenToCell(vec{ Fl::event_x(), Fl::event_y() });
        if (gameModel->getCell(cell.x, cell.y) == ALIVE) mouseToDead = true;
        else mouseToDead = false;

        if (mouseToDead) gameModel->setCell(cell.x, cell.y, DEAD);
        else gameModel->setCell(cell.x, cell.y, ALIVE);
        redraw();

        return 1;

    case FL_DRAG:
        if(lastMouseDrag.x != -1) {
            //cell = screenToCell(vec{ Fl::event_x(), Fl::event_y() });
            int cellSizeHalf;
            if (gameModel->getPixelDraw()) cellSizeHalf = (gameModel->getPixelZoom())/2;
            else cellSizeHalf = std::ceil(DEFAULT_CELL_SIZE/2.0f * gameModel->getZoom());
            if (cellSizeHalf == 0) cellSizeHalf = 1;
            int lineLength = (int)std::sqrt(std::pow(Fl::event_x() - lastMouseDrag.x, 2) + std::pow(Fl::event_y() - lastMouseDrag.y, 2));

            for (int i = 0; i < lineLength; i+=cellSizeHalf){
                cell = screenToCell(vec{(int)((Fl::event_x() - lastMouseDrag.x) * i/(float)lineLength) + lastMouseDrag.x,
                                        (int)((Fl::event_y() - lastMouseDrag.y) * i/(float)lineLength) + lastMouseDrag.y });

                if (mouseToDead) gameModel->setCell(cell.x, cell.y, DEAD);
                else gameModel->setCell(cell.x, cell.y, ALIVE);
            }
            redraw();
        }
        lastMouseDrag = vec{ Fl::event_x(), Fl::event_y() };

        return 1;

    case FL_RELEASE:
        lastMouseDrag = vec{ -1, -1 };

        return 1;

    case FL_FOCUS:
        return 1;

    case FL_UNFOCUS:
        return 1;

    default:
        return 0;
    }
}

vec GameView::screenToCell(vec pix) {
    vec viewCellPos;
    if (gameModel->getPixelDraw()) {
        double cellSize = gameModel->getPixelZoom();
        fvec pan = gameModel->getViewPan();
        vec topLeftCell = vec{ (int)(pan.x / DEFAULT_CELL_SIZE), (int)(pan.y / DEFAULT_CELL_SIZE) };
        viewCellPos = vec{ (int)(topLeftCell.x + pix.x/cellSize) , (int)(topLeftCell.y + pix.y / cellSize) };
    } else {
        double cellSize = DEFAULT_CELL_SIZE * gameModel->getZoom();
        fvec pan = gameModel->getViewPan();
        viewCellPos = vec{ (int)((pan.x + pix.x) / cellSize) , (int)((pan.y + pix.y) / cellSize) };
    }
    return viewCellPos;
}

int GameView::handleKB(int key) {
    bool shift = Fl::get_key(FL_Shift_L);
    bool ctrl = Fl::get_key(FL_Control_L);
    bool pixelDraw = gameModel->getPixelDraw();

    switch (key) {
    case 32: // 
        gameModel->toggleRunning();
        return 1;

    case 103: //g
        gameModel->toggleAcceleration();
        return 1;

    case 112: //p
        gameModel->togglePixelZoom();
        redraw();
        return 1;

    case 104: //h
        std::cout << "Controls:" << "\n";
        std::cout << "+/-            : Change simulation speed by 1" << "\n";
        std::cout << "+/- + shift    : Change simulation speed by 10" << "\n";
        std::cout << "+/- + ctrl     : Zoom (hold shift to zoom faster)" << "\n";
        std::cout << ". (dot)        : Steps the simulation once" << "\n";
        std::cout << ". + Shift      : Steps the simulation 10 times" << "\n";
        std::cout << ". + Ctrl Shift : Steps the simulation 1000 times" << "\n";
        std::cout << "Arrow keys     : Move around the grid (hold shift to move faster)" << "\n";
        std::cout << "Change the cell states by clicking/drawing with the mouse" << "\n";
        return 1;

    case 46: //.
        if (shift && ctrl) gameModel->StepSimulation(1000);
        else if (shift) gameModel->StepSimulation(10);
        else gameModel->StepSimulation(1);
        return 1;

    case 45: //-
        if (ctrl) {
            if (pixelDraw) {
                if (shift) gameModel->addPixelZoom(-5);
                else gameModel->addPixelZoom(-1);
            }
            else {
                if (shift) gameModel->addZoom(0.1);
                else gameModel->addZoom(0.5);
            }
            redraw();
        }
        else {
            if (shift) gameModel->addSimSpeed(-10);
            else gameModel->addSimSpeed(-1);
        }
    
        return 1;

    case 61: //+
        if (ctrl) {
            if (pixelDraw) {
                if (shift) gameModel->addPixelZoom(5);
                else gameModel->addPixelZoom(1);
            } else {
                if (shift) gameModel->addZoom(10);
                else gameModel->addZoom(2);
            }
            redraw();
        }
        else {
            if (shift) gameModel->addSimSpeed(10);
            else gameModel->addSimSpeed(1);
        }
        return 1;

    case 65453: //- numpad
        if (shift) gameModel->addSimSpeed(-10);
        else gameModel->addSimSpeed(-1);
        return 1;

    case 65451: //+ numpad
        if (shift) gameModel->addSimSpeed(10);
        else gameModel->addSimSpeed(1);
        return 1;
            
    case FL_Up:
        if (shift) gameModel->addViewPan(0, -10 * DEFAULT_CELL_SIZE * 0.5);
        else gameModel->addViewPan(0, -1 * DEFAULT_CELL_SIZE * 0.5);
        redraw();
        break;

    case FL_Down:
        if (shift) gameModel->addViewPan(0, 10 * DEFAULT_CELL_SIZE * 0.5);
        else gameModel->addViewPan(0, 1 * DEFAULT_CELL_SIZE * 0.5);
        redraw();
        break;

    case FL_Left:
        if (shift) gameModel->addViewPan(-10 * DEFAULT_CELL_SIZE * 0.5, 0);
        else gameModel->addViewPan(-1 * DEFAULT_CELL_SIZE * 0.5, 0);
        redraw();
        break;

    case FL_Right:
        if (shift) gameModel->addViewPan(10 * DEFAULT_CELL_SIZE * 0.5, 0);
        else gameModel->addViewPan(1 * DEFAULT_CELL_SIZE * 0.5, 0);
        redraw();
        break;

    default:
        return 0;
    }

}
