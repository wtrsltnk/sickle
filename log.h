#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>

void Log(const char *format, ...);

#ifdef LOG_H_IMPL

#define BUFFERSIZE 1024

void Log(const char *format, ...)
{
    va_list args;
    char buffer[BUFFERSIZE];

    va_start(args, format);
    vsprintf(buffer, format, args);

    FILE *file = fopen("log.txt", "a");
    if (file != 0)
	{
	    fwrite(buffer, strlen(buffer), 1, file);
	    fwrite("\n", 1, 1, file);
	    fclose(file);
	}
    va_end(args);
}

#endif // LOG_H_IMPL

#endif // _LOG_H_
