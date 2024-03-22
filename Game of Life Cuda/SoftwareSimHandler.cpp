
#include "SoftwareSimHandler.h"
#include <cstdlib>

int SoftwareSimStep(CellState** cellGridP, vec size, int steps) {

	CellState* inGrid = *cellGridP;
	CellState* outGrid = (CellState*) malloc(sizeof(CellState) * size.x * size.y);
	if (outGrid == NULL){
		return 1;
	}

	for (size_t i = 0; i < steps; i++){
		if (i != 0) {
			CellState* temp = inGrid;
			inGrid = outGrid;
			outGrid = temp;
		}
		
		for (size_t i = 0; i < size.x; i++)
		{
			for (size_t j = 0; j < size.y; j++)
			{
				int s = i * size.x + j;
				if (i == 0 || j == size.y - 1 || j == 0 || i == size.x - 1) {
					outGrid[i] = DEAD;
					continue;
				}

				int alive = inGrid[s - 1];
				alive += inGrid[s + 1];
				alive += inGrid[s - size.x - 1];
				alive += inGrid[s - size.x];
				alive += inGrid[s - size.x + 1];
				alive += inGrid[s + size.x - 1];
				alive += inGrid[s + size.x];
				alive += inGrid[s + size.x + 1];

				if (inGrid[s] == DEAD) {
					if (alive != ANIMATEDEAD) {
						outGrid[s] = DEAD;
						continue;
					}
				}
				else {
					if (alive < ISOLATION || alive > SMOTHERING) {
						outGrid[s] = DEAD;
						continue;
					}
				}
				outGrid[s] = ALIVE;
			}
		}
	}
	*cellGridP = outGrid;
	free(inGrid);
	return 0;
}