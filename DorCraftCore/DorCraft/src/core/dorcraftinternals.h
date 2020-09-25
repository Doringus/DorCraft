#pragma once

#include <stdio.h>

#define OUTPUT_COLOR_RED     "\x1b[31m"
#define OUTPUT_COLOR_GREEN   "\x1b[32m"
#define OUTPUT_COLOR_YELLOW  "\x1b[33m"
#define OUTPUT_COLOR_BLUE    "\x1b[34m"
#define OUTPUT_COLOR_MAGENTA "\x1b[35m"
#define OUTPUT_COLOR_CYAN    "\x1b[36m"
#define OUTPUT_COLOR_RESET   "\x1b[0m"

#define ERROR_LOG(log, ...) printf(OUTPUT_COLOR_RED log OUTPUT_COLOR_RESET, ##__VA_ARGS__)