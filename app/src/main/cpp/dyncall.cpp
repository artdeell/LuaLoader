//
// Created by maks on 04.01.2023.
//

#include "dyncall.h"


int lua_dodyncall(lua_State *L) {
    auto *pCallVM = (DCCallVM *) lua_touserdata(L, lua_upvalueindex(1));

    void *f;

    const char *callsignature, *s;

    int top = lua_gettop(L);

    if (top < 2) return luaL_error(L, "missing arguments #1 'addr' and #2 'signature'");


    if (lua_iscfunction(L, 1)) f = (void *) lua_tocfunction(L, 1);

    else if (lua_islightuserdata(L, 1)) f = lua_touserdata(L, 1);

    else if (lua_isnumber(L, 1)) f = (void *) lua_tointeger(L, 1);

    else return luaL_argerror(L, 1, "expected a cfunction, userdata or number");


    s = callsignature = luaL_checkstring(L, 2);


    /* parse mode */



    // dcMode( pCallVM, DC_CALL_C_DEFAULT );

    dcReset(pCallVM);


    char ch;

    int p = 3;

    int ptr = 0;

    while ((ch = *s++) != DC_SIGCHAR_ENDARG) {

        if (p > top)
            return luaL_error(L, "need more arguments (call signature is '%s')", callsignature);

        if (ptr == 0) {

            switch (ch) {

                case '*':

                    ptr++;

                    continue;

                case DC_SIGCHAR_BOOL:

                    dcArgBool(pCallVM, (DCbool) luaL_checkinteger(L, p));

                    break;

                case DC_SIGCHAR_CHAR:

                case DC_SIGCHAR_UCHAR:

                    dcArgChar(pCallVM, (DCchar) luaL_checkinteger(L, p));

                    break;

                case DC_SIGCHAR_SHORT:

                case DC_SIGCHAR_USHORT:

                    dcArgShort(pCallVM, (DCshort) luaL_checkinteger(L, p));

                    break;

                case DC_SIGCHAR_INT:

                case DC_SIGCHAR_UINT:

                    dcArgInt(pCallVM, (DCint) luaL_checknumber(L, p));

                    break;

                case DC_SIGCHAR_LONG:

                case DC_SIGCHAR_ULONG:

                    dcArgLong(pCallVM, (DClong) luaL_checknumber(L, p));

                    break;

                case DC_SIGCHAR_LONGLONG:

                case DC_SIGCHAR_ULONGLONG:

                    dcArgLongLong(pCallVM, (DClonglong) luaL_checknumber(L, p));

                    break;

                case DC_SIGCHAR_FLOAT:

                    dcArgFloat(pCallVM, (DCfloat) luaL_checknumber(L, p));

                    break;

                case DC_SIGCHAR_DOUBLE:

                    dcArgDouble(pCallVM, (DCdouble) luaL_checknumber(L, p));

                    break;

                case DC_SIGCHAR_POINTER:

                    dcArgPointer(pCallVM, (DCpointer) lua_topointer(L, p));

                    break;

                case DC_SIGCHAR_STRING:

                    dcArgPointer(pCallVM, (DCpointer) lua_tostring(L, p));

                    break;

                default:

                    return luaL_error(L, "invalid typecode '%c' in call signature '%s'", s[0],
                                      callsignature);

            }

        } else { /* pointer types */

            switch (ch) {

                case '*':

                    ptr++;

                    continue;

                case '<': {

                    const char *begin = s;

                    while ((ch = *s++) != '>');

                    const char *end = s;

                    switch (lua_type(L, p)) {

                        case LUA_TNUMBER:

                            dcArgPointer(pCallVM, (DCpointer) (ptrdiff_t) lua_tonumber(L, p));

                            break;

                        case LUA_TTABLE:

                            lua_pushvalue(L, p);        // 1

                            lua_pushliteral(L, "pointer");

                            lua_gettable(L, -2);        // 2

                            if (!lua_isuserdata(L, -1))

                                luaL_error(L, "pointer type mismatch at argument #%d", p);

                            dcArgPointer(pCallVM, (DCpointer) lua_touserdata(L, -1));

                            lua_pop(L, 2);

                            break;

                        case LUA_TLIGHTUSERDATA:

                        case LUA_TUSERDATA:

                            dcArgPointer(pCallVM, (DCpointer) lua_topointer(L, p));

                            break;

                        default:

                            luaL_error(L, "pointer type mismatch at argument #%d", p);

                            break;

                    }

                }

                    break;

                case DC_SIGCHAR_BOOL:

                case DC_SIGCHAR_CHAR:

                    if (lua_isstring(L, p)) {

                        dcArgPointer(pCallVM, (DCpointer) lua_tostring(L, p));

                        break;

                    }

                case DC_SIGCHAR_UCHAR:

                case DC_SIGCHAR_SHORT:

                case DC_SIGCHAR_USHORT:

                case DC_SIGCHAR_INT:

                case DC_SIGCHAR_UINT:

                case DC_SIGCHAR_LONG:

                case DC_SIGCHAR_ULONG:

                case DC_SIGCHAR_LONGLONG:

                case DC_SIGCHAR_ULONGLONG:

                case DC_SIGCHAR_FLOAT:

                case DC_SIGCHAR_DOUBLE:

                case DC_SIGCHAR_POINTER:

                case DC_SIGCHAR_STRING:

                case DC_SIGCHAR_VOID:

                    if (lua_istable(L, p)) {

                        lua_pushvalue(L, p);        // 1

                        lua_pushliteral(L, "pointer");

                        lua_gettable(L, -2);        // 2

                        if (!lua_isuserdata(L, -1))

                            luaL_error(L, "pointer type mismatch at argument #%d", p);

                        dcArgPointer(pCallVM, (DCpointer) lua_touserdata(L, -1));

                        lua_pop(L, 2);

                    } else

                        dcArgPointer(pCallVM, (DCpointer) lua_topointer(L, p));

                    ptr = 0;

                    break;

                default:

                    return luaL_error(L, "invalid signature");

            }

        }


        ++p;

    }


    if (top >= p)

        luaL_error(L, "too many arguments for given signature, expected %d but received %d", p - 3,
                   top - 2);


    switch (*s++) {

        case DC_SIGCHAR_VOID:

            dcCallVoid(pCallVM, f);

            return 0;

        case DC_SIGCHAR_BOOL:

            lua_pushboolean(L, (int) dcCallBool(pCallVM, f));

            break;

        case DC_SIGCHAR_CHAR:

        case DC_SIGCHAR_UCHAR:

            lua_pushnumber(L, (lua_Number) (dcCallChar(pCallVM, f)));

            break;

        case DC_SIGCHAR_SHORT:

        case DC_SIGCHAR_USHORT:

            lua_pushnumber(L, (lua_Number) (dcCallShort(pCallVM, f)));

            break;

        case DC_SIGCHAR_INT:

        case DC_SIGCHAR_UINT:

            lua_pushnumber(L, (lua_Number) (dcCallInt(pCallVM, f)));

            break;

        case DC_SIGCHAR_LONG:

        case DC_SIGCHAR_ULONG:

            lua_pushnumber(L, (lua_Number) (dcCallLong(pCallVM, f)));

            break;

        case DC_SIGCHAR_LONGLONG:

        case DC_SIGCHAR_ULONGLONG:

            lua_pushnumber(L, (lua_Number) (dcCallLongLong(pCallVM, f)));

            break;

        case DC_SIGCHAR_FLOAT:

            lua_pushnumber(L, (lua_Number) dcCallFloat(pCallVM, f));

            break;

        case DC_SIGCHAR_DOUBLE:

            lua_pushnumber(L, (lua_Number) dcCallDouble(pCallVM, f));

            break;

        case DC_SIGCHAR_STRING:

            lua_pushstring(L, (const char *) dcCallPointer(pCallVM, f));

            break;

        case DC_SIGCHAR_POINTER:

            lua_pushlightuserdata(L, dcCallPointer(pCallVM, f));

            break;

        case '*':

            switch (*s++) {

                case DC_SIGCHAR_UCHAR:

                case DC_SIGCHAR_CHAR:

                    lua_pushstring(L, (char *) dcCallPointer(pCallVM, f));

                    break;

                default:

                    lua_pushlightuserdata(L, dcCallPointer(pCallVM, f));

                    break;

            }

            break;

        default:

            return luaL_error(L, "invalid signature");

    }

    return 1;

}

void dyncall_register(lua_State *state) {
    lua_pushlightuserdata(state, dcNewCallVM(4096));
    lua_pushcclosure(state, &lua_dodyncall, 1);
    lua_setglobal(state, "dyncall");
}