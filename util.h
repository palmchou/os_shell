#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "parse.h"

void printPrompt();
int quit(int status);
char* alloc_str(const char* str);
char* get_history_file_path(char* path_value_keeper);
void printError(char* str, char* cmd);
char getch();
//char** getOptions(Command* command);

#endif