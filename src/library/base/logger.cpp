#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifndef LOG_DISABLED

#ifdef __unix__
#include <unistd.h>
#define MAX_PATH 255
#else
#include <windows.h>
#endif

static FILE *logFile = NULL;

static void timenow(char * buffer) {
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
}

static void getLogFname(char* logpath) {
#ifdef __unix__
	const char *folder = getenv("TMPDIR");
	if (folder == nullptr) {
		folder = "/tmp";
	}
	strncpy(logpath, folder, MAX_PATH);
	strncat(logpath, "/open-license.log", MAX_PATH - strlen(logpath));
#else
	wchar_t wlogpath[MAX_PATH];
	const int plen = GetTempPathW(MAX_PATH, wlogpath);
	if(plen == 0) {
		fprintf(stderr, "Error getting temporary directory path");
	} else {
		wcstombs(logpath, wlogpath, MAX_PATH);
	}
	strncat(logpath, "open-license.log", MAX_PATH - strlen(logpath));
#endif
}

void _log(const char* format, ...) {
	va_list args;
	char * buffer;
	if (logFile == NULL) {
		char logpath[MAX_PATH];
		getLogFname(logpath);
		logFile = fopen(logpath, "a");
		if (logFile == NULL) {
			return;
		}
	}
	buffer = (char *) malloc(sizeof(char) * strlen(format) + 64);
	timenow(buffer);
	strcat(buffer, format);
	va_start(args, format);
	vfprintf(logFile, buffer, args);
	va_end(args);
	free(buffer);
}

void _shutdown_log() {
	if (logFile != NULL) {
		fclose(logFile);
		logFile = NULL;
	}
}
#endif
