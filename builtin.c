//
// Created by Palm on 23/03/15.
//

#include "builtin.h"

int tryBuiltin(Command* command) {
    char* cmd = command->cmd_name;
    if (strcmp(cmd, "exit") == 0)
        builtinEXIT(command);
    else if (strcmp(cmd, "cd") == 0)
        builtinCD(command);
    else if (strcmp(cmd, "history") == 0)
        builtinHISTORY(command);
    else if (strcmp(cmd, "jobs") == 0)
        builtinJOBS(command);
    else if (strcmp(cmd, "continue") == 0)
        builtinCONTINUE(command);
    else if (strcmp(cmd, "fg") == 0)
        builtinFG(command);
    else if (strcmp(cmd, "bg") == 0)
        builtinBG(command);
    else if (strcmp(cmd, "status") == 0)
        builtinSTATUS(command);
    else
        return 1; // not a builtin command
    return 0;
}

int isBuiltin(Command* command) {
    char* cmd = command->cmd_name;
    if (strcmp(cmd, "exit") == 0)
        return 1;
    else if (strcmp(cmd, "cd") == 0)
        return 1;
    else if (strcmp(cmd, "history") == 0)
        return 1;
    else if (strcmp(cmd, "continue") == 0)
        return 1;
    else if (strcmp(cmd, "jobs") == 0)
        return 1;
    else if (strcmp(cmd, "fg") == 0)
        return 1;
    else if (strcmp(cmd, "bg") == 0)
        return 1;
    else if (strcmp(cmd, "status") == 0)
        return 1;
    else
        return 0; // not a builtin command
}


int builtinEXIT(Command* command) {
    quit(0);
    return 0;
}

int builtinCD(Command* command) {
    char* arg = *(command->argv + 1);

    errno = 0;
    if (arg != NULL) {
        chdir(*(command->argv + 1));
    } else {
        chdir(var_HOME);
    }
    if (errno != 0) {
        perror(NULL);
        errno = 0;
    }
    if(var_PWD != NULL)
        free(var_PWD);
    var_PWD = getcwd(NULL, 0);
    return 0;
}

int builtinHISTORY(Command* command) {
    int line_len = INPUT_LEN * 2; // multiplied two for safety
    char str[line_len];



    FILE* history_file_FD = fopen(var_HISTORY_FILE_PATH, "r");
    if (history_file_FD != NULL) { // open file success
        while (fgets(str, line_len, history_file_FD) != NULL) {
            printf("%s", str);
        }
        fclose(history_file_FD);
        return 0;
    } else if (tmp_HISTORY_FD != NULL){ // open file failed,
        rewind(tmp_HISTORY_FD);
        while (fgets(str, line_len, tmp_HISTORY_FD) != NULL) {
            printf("%s", str);
        }
        return 0;
    } else { // neither history file nor temp history file was opened successfully
        fprintf(stderr, "printing commands history failed");
        return 1;
    }
}

int builtinCONTINUE(Command* command) {
    char* job_id_str = command->argv[1];
    if (job_id_str != NULL) {
        if(job_id_str[0] == '%') {
            unsigned long jid = strtoul(job_id_str + 1, NULL, 10);
            if (jid > 0 && jid < MAX_JOBS_NUM) {
                Job* job = Jobs_table + jid;
                if (job->runSTAT == STOPED) {
                    if (kill(job->pid, SIGCONT) == 0) {// signal sent
                        int status;
                        current_job = job;
                        job->runSTAT = RUNNING;
                        job->groundType = FG;
                        waitpid(job->pid, &status, WUNTRACED);
                        if (!WIFSTOPPED(status)) {
                            reset_job_position(current_job);
                        }
                        current_job = NULL;
                        return 0;
                    } else {
                        perror("continue");
                    }
                } else {
                    fprintf(stderr, "job with jid = %lu was not stopped\n", jid);
                }
            } else {
                fprintf(stderr, "job with jid = %lu was not found\n", jid);
            }
        } else {
            fprintf(stderr, "usage: continue %%JID\nPlease do not forget the percent sign.\n");
        }
    } else {
        fprintf(stderr, "usage: continue %%JID\n");
    }
    return -1;
}

int builtinJOBS(Command* command) {
    int i;
    for (i = 1; i < MAX_JOBS_NUM; i++) {
        Job* job = Jobs_table + i;
        if (job->active)
            print_job_info(Jobs_table + i);
    }
    return 0;
}

int builtinFG(Command* command) {
    char* job_id_str = command->argv[1];
    if (job_id_str != NULL) {
        if(job_id_str[0] == '%') {
            unsigned long jid = strtoul(job_id_str + 1, NULL, 10);
            if (jid > 0 && jid < MAX_JOBS_NUM) {
                Job* job = Jobs_table + jid;
                if (job->runSTAT == STOPED) {
                    if (kill(job->pid, SIGCONT) == 0) {// signal sent
                        int status;
                        current_job = job;
                        job->runSTAT = RUNNING;
                        job->groundType = FG;
                        waitpid(job->pid, &status, WUNTRACED);
                        if (!WIFSTOPPED(status)) {
                            reset_job_position(current_job);
                        }
                        current_job = NULL;
                        return 0;
                    } else {
                        perror("fg");
                    }
                } else {
                    int status;
                    current_job = job;
                    job->runSTAT = RUNNING;
                    job->groundType = FG;
                    waitpid(job->pid, &status, WUNTRACED);
                    if (!WIFSTOPPED(status)) {
                        reset_job_position(current_job);
                    }
                    current_job = NULL;
                    return 0;
                }
            } else {
                fprintf(stderr, "job with jid = %lu was not found\n", jid);
            }
        } else {
            fprintf(stderr, "usage: fg %%JID\nPlease do not forget the percent sign.\n");
        }
    } else {
        fprintf(stderr, "usage: fg %%JID\n");
    }
    return -1;
}

int builtinBG(Command* command) {
    char* job_id_str = command->argv[1];
    if (job_id_str != NULL) {
        if(job_id_str[0] == '%') {
            unsigned long jid = strtoul(job_id_str + 1, NULL, 10);
            if (jid > 0 && jid < MAX_JOBS_NUM) {
                Job* job = Jobs_table + jid;
                if (job->runSTAT == STOPED) {
                    if (kill(job->pid, SIGCONT) == 0) {// signal sent
                        job->runSTAT = RUNNING;
                        job->groundType = BG;
                        current_job = NULL;
                        return 0;
                    } else {
                        perror("bg");
                    }
                } else {
                    fprintf(stderr, "job with jid = %lu was not stopped\n", jid);
                }
            } else {
                fprintf(stderr, "job with jid = %lu was not found\n", jid);
            }
        } else {
            fprintf(stderr, "usage: bg %%JID\nPlease do not forget the percent sign.\n");
        }
    } else {
        fprintf(stderr, "usage: bg %%JID\n");
    }
    return -1;
}
int builtinSTATUS(Command* command) {
    printf("%d\n", last_running_status);
    return 0;
}