#ifndef _TUILE_H_
#define _TUILE_H_

#include <stdio.h>
#include <stdlib.h>

#include "utils_v1.h"

/**
 * Generate a random tile
*/
int *genTuile();

/**
 * Sort player by score
*/
void sortPlayersByScore(Player *players, int nbPlayers);

#endif // _TUILE_H_
