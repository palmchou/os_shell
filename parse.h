#ifndef PARSE_H
#define PARSE_H

#include <ctype.h>
#include "structures.h"
#include "shell.h"

extern char* input;

TokenNode* tokenList;
Command* commandList;

int parseInput();
int scan();
int parseTokenList();
TokenNode* createToken(TokenType tType, char* str);
TokenNode* appendTokenNode(TokenNode* parent, TokenNode* child);
Command* constructOneCMD(int* status);
Command* allocCMD(char* cmd_name);



void freeToken(TokenNode* token);
void freeTokenList(TokenNode* token);
void freeCommand(Command* cmd);
void freeCommandList();

int isRedirectionType(TokenType tokenType);
int getRdTypeFromTokenType(TokenType tokenType);
#endif