#pragma once
#include <stdint.h>

double octavePerlin(double x, double z, uint16_t octaves);
void perlinSeed(double seed);