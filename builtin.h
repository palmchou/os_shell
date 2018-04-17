//
// Created by Palm on 23/03/15.
//

#ifndef _SHELL_BUILTIN_H_
#define _SHELL_BUILTIN_H_

#include <string.h>
#include <errno.h>
#include "structures.h"
#include "util.h"

int tryBuiltin(Command* command);
int isBuiltin(Command* command);

int builtinEXIT(Command* command);
int builtinCD(Command* command);
int builtinHISTORY(Command* command);
int builtinCONTINUE(Command* command);
int builtinJOBS(Command* command);
int builtinFG(Command* command);
int builtinBG(Command* command);
int builtinSTATUS(Command* command);
#endif //_SHELL_BUILTIN_H_
