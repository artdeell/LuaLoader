//
// Created by maks on 04.01.2023.
//

#ifndef USERLIB_DYNCALL_H
#define USERLIB_DYNCALL_H
#include "lua-include.hpp"
#include "dyncall-1.4/dyncall/dyncall.h"
#include "dyncall-1.4/dyncall/dyncall_signature.h"

void dyncall_register(lua_State *state);

#endif //USERLIB_DYNCALL_H
