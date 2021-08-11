#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

#include <stdint.h>

double octavePerlin(double x, double z, uint16_t octaves);
void perlinSeed(double seed);

#endif