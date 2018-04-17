#include "parse.h"


int parseInput() {
    if (scan() != 0)
        return 1;

    if (parseTokenList() != 0)
        return 2;
    if (commandList == NULL)
        return 3; // means no commands.
    return 0;
}

int scan() {
    char ch;
    char str[INPUT_LEN+1];
    int i = 0; // index for char array 'input'
    int j = 0; // index for char array 'str'
    TokenNode* currentToken = tokenList = NULL;
    ch = input[i];
    while (ch != '\0') {
        if (isspace(ch) || ch == '&') { // isspace() contains '\n', while isblank() does not
            ;
        } else if (ch == '|') { // read a pipe
            currentToken = appendTokenNode(currentToken, createToken(PIPE, NULL));
        } else if (ch == '<') {
            currentToken = appendTokenNode(currentToken, createToken(IN_R, NULL));
        } else if (ch == '>') {
            if (input[i+1] == '>') {
                i++; // move the ch pointer
                currentToken = appendTokenNode(currentToken, createToken(OUT_R_A, NULL));
            } else { // input[i+1] != '>'
                currentToken = appendTokenNode(currentToken, createToken(OUT_R, NULL));
            }
        } else if (isalnum(ch) || ch == '/' || ch == '.' || ch == '-' || ch == '_' || ch == '~' || ch == '%') {
            j = 0;
            while (isalnum(ch)  || ch == '/' || ch == '.' || ch == '-' || ch == '_' || ch == '~' || ch == '%') {
                str[j++] = ch;
                ch = input[++i];
            }
            i--;
            str[j] = '\0';
            currentToken = appendTokenNode(currentToken, createToken(WORD, alloc_str(str)));
        } else { // wrong state
            puts("Wrong input, read illegal character");
            return 1;
        }

        ch = input[++i];
    }
    return 0;
}

int parseTokenList() {
    int status = 0;
    TokenNode* temp;
    Command* cmd = NULL;
    Command* last = NULL;
    while (tokenList != NULL) {
        if (commandList == NULL) {
            if ((cmd = constructOneCMD(&status)) != NULL) {
                commandList = cmd;
                last = commandList;
            } else {
                return 1; // parsing tokens to command failed
            }
        } else if (tokenList->type == PIPE) { // commandList is not empty, and next token is a pipe
            temp = tokenList;
            tokenList = tokenList->next;
            freeToken(temp);
            if ((cmd = constructOneCMD(&status)) != NULL) {
                last->next = cmd;
                last = last->next;
            } else {
                return 1;
            }
        } else { // have parsed some but the left in tokenList is something useless
            freeTokenList(tokenList);
        }
    }
    return 0;
}

TokenNode* createToken(TokenType tType, char* str) {
    TokenNode* tNode = (TokenNode*)malloc(sizeof(TokenNode));
    if (tNode != NULL) {
        tNode->type = tType;
        tNode->str = str;
        tNode->next = NULL;
        return tNode;
    } else {
        puts("Memory allocation failed. Exit the shell.");
        exit(1);
    }
}

TokenNode* appendTokenNode(TokenNode* parent, TokenNode* child) {
    if (parent == NULL) {
        tokenList = child;
    } else {
        parent->next = child;
    }
    return child;
}

//
Command* constructOneCMD(int* status) {
    TokenNode* token = tokenList;
    Command* command = NULL;
    int arg_count = 0;
    int arg_index = 0;
    char** argv = NULL;
    if (token->type != WORD) {
        printError("Wrong input, expecting a WORD", NULL);
        return NULL;
    }
    command = allocCMD(alloc_str(token->str));
    tokenList = token->next;
    freeToken(token);
    token = tokenList;
    while (token != NULL && token->type == WORD) { // count arguments numbers.
        arg_count++;
        token = token->next;
    }
    argv = calloc((size_t)(arg_count + 2), sizeof(char*)); // create arguments array.
    *(argv + arg_index++) = alloc_str(command->cmd_name);
    token = tokenList;
    while (token != NULL && token->type == WORD) {
        if (token->str[0] == '~') { //
            char* str;
            size_t len = strlen(var_HOME);
            len += strlen(token->str) - 1;
            str = calloc(len + 1, sizeof(char));
            str[0] = '\0';
            strcat(str, var_HOME);
            strcat(str, (token->str + 1));
            *(argv + arg_index++) = str;
        } else {
            *(argv + arg_index++) = alloc_str(token->str);
        }
        tokenList = token->next;
        freeToken(token);
        token = tokenList;
    }
    *(argv + arg_index) = NULL;
    command->argv = argv;

    // redirection
    while (token != NULL && isRedirectionType(token->type)) {
        if (token->next->type == WORD) { // next token is a WORD, correct syntax
            if (getRdTypeFromTokenType(token->type) == IN) { // redirection in
                command->rdInType = IN;
                command->redirect_from = alloc_str(token->next->str);
            } else { // redirection out
                command->rdOutType = (RdOutType)getRdTypeFromTokenType(token->type);
                command->redirect_to = alloc_str(token->next->str);
            }
        } else { // syntax error, expecting a WORD
            *status = 1;
            freeTokenList(tokenList);
            freeCommand(command);
            printError("syntax error.", NULL);
            return NULL;
        }

        // read two tokens successfully, now free them
        tokenList = token->next->next;
        freeToken(token->next);
        freeToken(token);

        // ignore all the left token with type of WORD, in case if user inputted too much of it.
        token = tokenList;
        while(token != NULL && token->type == WORD) {
            tokenList = token->next;
            freeToken(token);
            token = tokenList;
        }
    }
    *status = 0;
    return command;
}



Command* allocCMD(char* cmd_name) {
    Command* cmd = malloc(sizeof(Command));
    if (cmd != NULL) {
        cmd->cmd_name = cmd_name;
        cmd->argv = NULL;
        cmd->next = NULL;
        cmd->rdInType = IN_NIL;
        cmd->rdOutType = OUT_NIL;
        cmd->redirect_to = NULL;
        cmd->redirect_from = NULL;
    }
    return cmd;
}

void freeToken(TokenNode* token) {
    if (token == NULL)
        return;
    if (token->str != NULL) {
        free(token->str);
    }
    free(token);
    return;
}

void freeTokenList(TokenNode* token) {
    TokenNode* tempNode;
    while (token != NULL) {
        tempNode = token;
        token = token->next;
        freeToken(tempNode);
    }
}

void freeCommand(Command* cmd) {
    if (cmd != NULL) {
        char** argv;
        int arg_index;
        if (cmd->cmd_name != NULL)
            free(cmd->cmd_name);
        if (cmd->redirect_to != NULL)
            free(cmd->redirect_to);
        if (cmd->redirect_from != NULL)
            free(cmd->redirect_from);
        argv = cmd->argv;
        for(arg_index = 0; *(argv + arg_index) != NULL; arg_index++) {
            free(*(argv + arg_index));
        }
        free(argv);
        free(cmd);
    }
}

void freeCommandList() {
    Command* ctemp;
    while (commandList != NULL) {
        ctemp = commandList;
        commandList = commandList->next;
        freeCommand(ctemp);
    }
    commandList = NULL;
}

int isRedirectionType(TokenType tokenType) {
    switch (tokenType) {
        case OUT_R:
        case OUT_R_A:
        case IN_R:
            return 1;
        default:
            return 0;
    }
}

int getRdTypeFromTokenType(TokenType tokenType) {
    switch (tokenType) {
        case OUT_R:
            return OUT;
        case OUT_R_A:
            return OUT_A;
        case IN_R:
            return IN;
        default:
            return 0; // both IN_NIL and OUT_NIL have constant value 0
    }
}