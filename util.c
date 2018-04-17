#include "util.h"


void printPrompt() {
    size_t i;
    char ch;
    char* prompt_path;
    i = strlen(var_PWD) -1;
    for (ch = var_PWD[i]; ch != '/'; ch = var_PWD[--i]); // find the last / symbol
    if (i != 0) { // not the directories in root directory or root directory
        i++;
    }
    if (strcmp(var_PWD, var_HOME) == 0) { // the cwd is the same as home path, just print ~ symbol for short
        prompt_path = "~";
    } else {
        prompt_path = &var_PWD[i];
    }
    printf("%s $ ", prompt_path);
    fflush(stdout);
}

int quit(int status) {
    if (input != NULL) {
        free(input);
    }
    if (commandList != NULL) {
        freeCommandList();
    }
    if (tmp_HISTORY_FD != NULL) {
        fclose(tmp_HISTORY_FD);
    }
    if (var_HISTORY_FILE_PATH != NULL) {
        free(var_HISTORY_FILE_PATH);
    }
    if (var_PWD != NULL) {
        free(var_PWD);
    }
    if (var_HOME != NULL) {
        free(var_HOME);
    }
    switch (status) {
        case 0:
            exit(0);
        case 10:
            fprintf(stderr, "Run out of jobs room.\n");
            fprintf(stderr, "The maximal amount of jobs is %d.\n", MAX_JOBS_NUM);
            exit(10);
        default:
            puts("Unknow error now quit.");
            exit(-1);
    }
}

char* alloc_str(const char* str) {
    if (str == NULL)
        return NULL;
    char* new_str = malloc((strlen(str) + 1) * sizeof(char));
    strcpy(new_str, str);
    return new_str;
}

char* get_history_file_path(char* path_value_keeper) {
    size_t len = strlen(var_HOME) + strlen(history_file_name + 1); // history_file_name is defined in init() in main.c
    char history_file_path[len + 1];
    strcpy(history_file_path, var_HOME);
    strcat(history_file_path, "/");
    strcat(history_file_path, history_file_name);
    if (DEBUG) {
        puts("history file path:");
        puts(history_file_path);
    }
    path_value_keeper = alloc_str(history_file_path);
    return path_value_keeper;
}

void printError(char* str, char* cmd) {
    if (cmd == NULL) {
        fprintf(stderr,"osshell: %s\n", str);
    } else {
        fprintf(stderr,"osshell: %s: %s\n", str, cmd);
    }

}

/* Read 1 character - echo defines echo mode */
//char getch()
//{
//    char ch;
//    static struct termios old, new;
//    /* Initialize new terminal i/o settings */
//    tcgetattr(0, &old); /* grab old terminal i/o settings */
//    new = old; /* make new settings same as old settings */
//    new.c_lflag &= ~ICANON; /* disable buffered i/o */
//    new.c_lflag &= ~ECHO; /* set echo mode */
//    tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
//    ch = (char)getchar();
//    /* Restore old terminal i/o settings */
//    tcsetattr(0, TCSANOW, &old);
//    return ch;
//}

char getch(){
    /*#include <unistd.h>   //_getch*/
    /*#include <termios.h>  //_getch*/
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    return buf;
}