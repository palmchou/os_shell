#include "shell.h"



int init(int argc, char** argv, char** env) { // should to be called at the start stage of shell and only called once
    input_len = INPUT_LEN;
    input = (char* )malloc(input_len + 1);
    struct passwd* pw = getpwuid(getuid());

    DEBUG = 0;
    if(argc == 2) {
        if (strcmp(argv[1], "--verbose") == 0)
            DEBUG = 1;
    }

    history_file_name = ".osshell_history";

    var_HOME = alloc_str(pw->pw_dir);
    var_PWD = alloc_str((var_HOME));
    var_HISTORY_FILE_PATH = get_history_file_path(var_HISTORY_FILE_PATH);
    tmp_HISTORY_FD = tmpfile(); // open a temp file to store the history temporarily. This FD will be close in quit() in util.c

    chdir(var_PWD);

    // phase 3
    init_job_control();

    last_running_status = 0;

    return 0;
}

int main(int argc, char** argv, char** env) {
    init(argc, argv, env);

    mainLoop();
    quit(2);
    return -1;
}

int mainLoop() {
    printPrompt();
    while(readFromStdin() != -1) {
        write_history();
        if (parseInput() == 0) {
            execCommand();
        }
        freeCommandList();
        printPrompt();
    }
    return -1;
}

// read line from stdin, and delete the last new line character.
int readFromStdin() {
    int bytes_read;
    if ((bytes_read = (int)getline(&input, &input_len, stdin)) != -1) {
        if (input[strlen(input) -1 ] == '\n')
            input[strlen(input) -1] = 0;
        return bytes_read;
    } else {
        puts("Something wrong with getline()");
        return -1; // something wrong
    }
}

int execCommand() {
    Command* command = commandList;
    int initial_in;
    int initial_out;
    int rd_in = -1;
    int rd_out = -1;
    int fd_in;
    int p[2];
    int jid;
    Job* job;
    int status;
    int bg_flag;

    if (command == NULL)
        return 1; // NULL command



    job = NULL;
    if (!isBuiltin(command)) {
        jid = getLeastAvailableJID();
        job = current_job = (Jobs_table + jid);
        job->jid = jid;
        job->runSTAT = RUNNING;
        job->groundType = FG;
        job->job_command = alloc_str(input);
        job->active = 1;
    }

    bg_flag = (input[strlen(input) - 1] == '&') ? 1 : 0;


    initial_in = dup(STDIN_FILENO);
    initial_out = dup(STDOUT_FILENO);


    fd_in = STDIN_FILENO;
    while (command != NULL) {
        status = 0;
        if (command == commandList) { // the first command
            if (command->rdInType != IN_NIL) { // may have input redirection
                rd_in = redirect_from(command->redirect_from);
                if (rd_in < 0)
                    return 3; // fail to redirect in
                else
                    fd_in = rd_in;
            }
        }
        if (command->next == NULL) { // the last command
            if (command->rdOutType != OUT_NIL) {
                switch (command->rdOutType) {
                    case OUT:
                        if ((rd_out = redirect_to(command->redirect_to, 0)) < 0) {
                            return 4; // fail to redirect out
                        }
                        break;
                    case OUT_A:
                        if ((rd_out = redirect_to(command->redirect_to, 1)) < 0) {
                            return 5; // fail to redirect out in append mod
                        }
                        break;
                    case OUT_NIL: // wont fall through ever, just to eliminate the compile warning.
                        break;
                }
            }
        }


        pipe(p);
        mySystem(command, &fd_in, p, job, &status, bg_flag);


        if (command == commandList) { // the first command
            if (rd_in != -1) { // if had redirected
                close(rd_in);
                if (dup2(initial_in, STDIN_FILENO) < 0) {
                    printError("Fail to reset redirection.", NULL);
                    return 6;
                }
            }
        }
        if (command->next == NULL) { // the last commmand
            if (rd_out != -1) {
                close(rd_out);
                if (dup2(initial_out, STDOUT_FILENO) < 0) {
                    printError("Fail to reset redirection.", NULL);
                    return 7;
                }
            }
        }

        command = command->next;
    }

    close(initial_in);
    close(initial_out);

    if (fd_in != STDIN_FILENO) {
        close(fd_in);
    }
    current_job = NULL;
    return 0;
}

int mySystem(Command* cmd, int* fd_in, int pipe_fd[2], Job* job, int* status, int bg_flag) {
    int pid;
    if (cmd == NULL) {
        close(pipe_fd[0]);
        close(pipe_fd[0]);
        return -1;
    }
    if (isBuiltin(cmd)) {
        dup2(*fd_in, STDIN_FILENO);
        if (cmd->next != NULL) { // has next command, need to pipe out
            int dup_out = dup(STDOUT_FILENO);
            dup2(pipe_fd[1], STDOUT_FILENO);
            tryBuiltin(cmd);
            close(pipe_fd[1]);
            dup2(dup_out, STDOUT_FILENO);
            *fd_in = pipe_fd[0];
        } else {
            tryBuiltin(cmd);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
        }
        return 0;
    } else {
        if ((pid = fork()) < 0) {
            perror("Fork failed");
            return -2;
        }
        if (pid == 0) { // child
//            if (setpgid(0, getpid()) != 0)
//                perror("setpgid");

            dup2(*fd_in, STDIN_FILENO);
            close(pipe_fd[0]); //close reading end, for not wasting FDs
            if (cmd->next != NULL) { // has next command, need to pipe out
                dup2(pipe_fd[1], STDOUT_FILENO);
            } else {
                close(pipe_fd[1]);
            }
            execvp(cmd->cmd_name, cmd->argv);
            if (errno == ENOENT) {
                printError("command not found.", cmd->cmd_name);
            } else {
                perror("osshell");
            }
            exit(-1);
        } else {
            job->pid = pid;
            if (bg_flag) {
                job->groundType = BG;
                job->runSTAT = RUNNING;
                print_job_info(job);
                current_job = NULL;
            } else {
                waitpid(pid, status, WUNTRACED);
                if (!WIFSTOPPED(*status)) {
                    reset_job_position(current_job);
                }
                if (WIFEXITED(status)) {
                    last_running_status = WEXITSTATUS(status);
                }
            }
            close(pipe_fd[1]);
            if (*fd_in != STDIN_FILENO) {
                close(*fd_in);
            }
            *fd_in = pipe_fd[0];
            return 0;
        }
    }
}

int write_history() {
    FILE* history_file_FD = fopen(var_HISTORY_FILE_PATH, "a");
    if (history_file_FD != NULL) { // open history file success
        fputs(input, history_file_FD);
        fputs("\n", history_file_FD);
        fclose(history_file_FD);
    }
    if (tmp_HISTORY_FD != NULL) {
        fseek(tmp_HISTORY_FD, 0, SEEK_END);
        fputs("\n", tmp_HISTORY_FD);
        fputs(input, tmp_HISTORY_FD);
    }
    return 0;
}

int redirect_from(char* file_name) {
    FILE* in = fopen(file_name, "r");
    if (dup2(fileno(in), STDIN_FILENO) < 0) {
        printError("Fail to redirect in.", NULL);
        return -1;
    }
    return fileno(in);
}

int redirect_to(char* file_name, int append) {
    FILE* out;
    if (append) {
        out = fopen(file_name, "a");
    } else {
        out = fopen(file_name, "w");
    }
    if (dup2(fileno(out), STDOUT_FILENO) < 0) {
        printError("Fail to redirect out", NULL);
        return -1;
    }
    return fileno(out);
}