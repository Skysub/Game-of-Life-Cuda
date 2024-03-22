#pragma once
#ifndef	CSH_H
#define CSH_H

#include "GameModel.h"

int SimStep(CellState* cellGrid, vec size, int steps);
int CudaCleanup();

#endif