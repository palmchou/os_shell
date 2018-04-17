//
// Created by Palm on 22/05/15.
//

#ifndef SHELL_JOB_CONTROL_H
#define SHELL_JOB_CONTROL_H

#include <signal.h>
#include <stdio.h>
#include "util.h"
#include "structures.h"

Job* Jobs_table;
Job* current_job;

void signal_handler(int sig);
void init_job_control();
unsigned int getLeastAvailableJID();
void reset_job_position(Job* job);
void print_job_info(Job* job);
Job* find_job_for(int pid);

#endif //SHELL_JOB_CONTROL_H
