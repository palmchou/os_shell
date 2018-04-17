//
// Created by Palm on 23/03/15.
//

#ifndef _SHELL_STRUCTURES_H_
#define _SHELL_STRUCTURES_H_

typedef enum {
    WORD, PIPE,
    IN_R, // input redirection
    OUT_R, OUT_R_A // output redirection and output redirection append
} TokenType;

typedef struct tokenNode {
    TokenType type;
    char* str;
    struct tokenNode* next;
} TokenNode;

typedef enum rdInType {
    IN_NIL = 0, IN = 1
} RdInType;

typedef enum rdOutType {
    OUT_NIL = 0, OUT = 2, OUT_A = 3
} RdOutType;

typedef struct command {
    char* cmd_name;
    char** argv;
    RdInType rdInType;
    RdOutType rdOutType;

    char* redirect_to;
    char* redirect_from;
    struct command* next;
} Command;

typedef enum runningStatus {
    RunSTAT_NIL, RUNNING, STOPED
} RunSTAT;

typedef enum ground {
    GroundType_NIL, FG, BG
} GroundType;


typedef struct job_struct {
    int pid;
    int jid;
    int active;
    RunSTAT runSTAT;
    GroundType groundType;
    char* job_command;
} Job;

#endif //_SHELL_STRUCTURES_H_
