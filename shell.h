#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "structures.h"
#include "util.h"
#include "parse.h"
#include "builtin.h"
#include "job_control.h"
#define INPUT_LEN 255
#define MAX_JOBS_NUM 5
#define NOT_ENOUGH_MEMORY 9814

int DEBUG;

char* var_HOME;
char* var_PWD;
char* var_HISTORY_FILE_PATH;
FILE* tmp_HISTORY_FD;

char* history_file_name;

char* input;
size_t input_len;
int last_running_status;


int readFromStdin();
int write_history();
int mySystem(Command* cmd, int* fd_in, int pipe_fd[2], Job* job, int* status, int);
int mainLoop();
int init(int argc, char** argv, char** env);
int execCommand();

int redirect_from(char* file_name);
int redirect_to(char* file_name, int append);

#endif