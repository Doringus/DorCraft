#pragma once

#include <stdio.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define OUTPUT_COLOR_RED     "\x1b[31m"
#define OUTPUT_COLOR_GREEN   "\x1b[32m"
#define OUTPUT_COLOR_YELLOW  "\x1b[33m"
#define OUTPUT_COLOR_BLUE    "\x1b[34m"
#define OUTPUT_COLOR_MAGENTA "\x1b[35m"
#define OUTPUT_COLOR_CYAN    "\x1b[36m"
#define OUTPUT_COLOR_RESET   "\x1b[0m"

#define ERROR_LOG(log, ...) printf(OUTPUT_COLOR_RED log OUTPUT_COLOR_RESET "\n", ##__VA_ARGS__)
#define WARNING_LOG(log, ...) printf(OUTPUT_COLOR_YELLOW log OUTPUT_COLOR_RESET "\n", ##__VA_ARGS__)

#define KILOBYTES(value) ((value) * 1024)
#define MEGABYTES(value) (KILOBYTES(value) * 1024)
#define GIGABYTES(value) (MEGABYTES(value) * 1024)

static void* readFile(char *filename) {
	void* result = 0;
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			result = VirtualAlloc(0, (SIZE_T)fileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result) {
				DWORD bytesRead;
				if (ReadFile(fileHandle, result, fileSize.QuadPart, &bytesRead, 0)
					&& fileSize.QuadPart == bytesRead) {
					// File read successfully
				}
				else {
					ERROR_LOG("Cannot read file %s", filename);
				}
			}
			else {
				ERROR_LOG("Cannot alloc memory for file data %s", filename);
			}
		}
		else {
			ERROR_LOG("Cannot get file size %s", filename);
		}
	}
	else {
		ERROR_LOG("Cannot open file for reading %s", filename);
	}
	return(result);
}

static bool writeFile(char *filename, unsigned int memSize, void *memory) {
	bool result = false;
	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memSize, &bytesWritten, 0)) {
			result = (bytesWritten == memSize);
		}
		else {
			ERROR_LOG("Cannot write to file %s", filename);
		}
	}
	else {
		ERROR_LOG("Cannot open file %s", filename);
	}
	return result;
}