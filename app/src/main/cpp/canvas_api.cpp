//
// Created by maks on 03.01.2023.
//

#include <android/log.h>
#include "canvas_api.h"
#include "includes/cipher/Cipher.h"


static int cipher_getGameVersion(lua_State *state) {
    lua_pushnumber(state, Cipher::getGameVersion());
    return 1;
}
static int cipher_isGameBeta(lua_State *state) {
    lua_pushboolean(state, Cipher::isGameBeta());
    return 1;
}
static int cipher_getLibBase(lua_State *state) {
    lua_pushlightuserdata(state, (void*)Cipher::get_libBase());
    return 1;
}
static int cipher_getLibName(lua_State *state) {
    lua_pushstring(state, Cipher::get_libName());
    return 1;
}
static int cipher_Scan(lua_State *state) {
    const char* pattern = luaL_checkstring(state, 1);
    const char* mask = luaL_checkstring(state, 2);
    uintptr_t scan_result = Cipher::CipherScan(pattern, mask);
    if(scan_result == 0) lua_pushnil(state);
    else lua_pushlightuserdata(state, (void*)scan_result);
    return 1;
}
static int cipher_ScanRange(lua_State *state) {
    const auto start = (uintptr_t) lua_touserdata(state, 1);
    const auto length = (size_t) lua_touserdata(state, 2);
    if(start == 0 || length == 0) {
        lua_pushnil(state);
        lua_pushstring(state, "Start or length are not valid. Did you supply them as userdata?");
        lua_error(state);
        return 2;
    }
    const char* pattern = luaL_checkstring(state, 3);
    const char* mask = luaL_checkstring(state, 4);
    uintptr_t scan_result = Cipher::CipherScan(start, length, pattern, mask);
    if(scan_result == 0) lua_pushnil(state);
    else lua_pushlightuserdata(state, (void*)scan_result);
    return 1;
}

static const luaL_Reg cipher_functions[] {
        {"getGameVersion", &cipher_getGameVersion},
        {"isGameBeta", &cipher_isGameBeta},
        {"get_libBase", &cipher_getLibBase},
        {"get_libName", &cipher_getLibName},
        {"CipherScan", &cipher_Scan},
        {"CipherScanRange", &cipher_ScanRange},
        {nullptr, nullptr}
};

static int lu_fromhex(lua_State *state) {
    const char* adr_hex = luaL_checkstring(state, 1);
    char* endptr;
    uintptr_t result = strtoul(adr_hex, &endptr, 16);
    if(*endptr != '\0') {
        lua_pushnil(state);
        lua_pushstring(state, "Not a hex number");
        lua_error(state);
        return 2;
    }
    lua_pushlightuserdata(state, (void*)result);
    return 1;
}
static int lu_add(lua_State *state) {
    if(lua_islightuserdata(state, 1)) {
        auto ptr = (uintptr_t) lua_touserdata(state, 1);
        ptr += luaL_checkinteger(state, 2);
        lua_pushlightuserdata(state, (void *)ptr);
        return 1;
    }else {
        lua_pushnil(state);
        lua_pushstring(state, "First arg is not a userdata");
        lua_error(state);
        return 2;
    }
}
static int lu_multiply(lua_State *state) {
    if(lua_islightuserdata(state, 1)) {
        auto ptr = (uintptr_t) lua_touserdata(state, 1);
        ptr *= luaL_checkinteger(state, 2);
        lua_pushlightuserdata(state, (void *)ptr);
        return 1;
    }else {
        lua_pushnil(state);
        lua_pushstring(state, "First arg is not a userdata");
        lua_error(state);
        return 2;
    }
}

static int lu_divide(lua_State *state) {
    if(lua_isuserdata(state, 1)) {
        auto ptr = (uintptr_t) lua_touserdata(state, 1);
        ptr = ptr / luaL_checkinteger(state, 2);
        lua_pushlightuserdata(state, (void *)ptr);
        return 1;
    }else {
        lua_pushnil(state);
        lua_pushstring(state, "First arg is not a userdata");
        lua_error(state);
        return 2;
    }
}

static int lu_readptr(lua_State *state) {
    luaL_checktype(state, 1, LUA_TLIGHTUSERDATA);
    lua_pushlightuserdata(state, *(void**)(lua_touserdata(state, 1)));
    return 1;
}
#define writerfunc(TYPENAME, MAX, MIN) \
static int lu_write##TYPENAME(lua_State *state) { \
    if(lua_gettop(state) < 2) { \
        luaL_error(state, "not enough arguments (expected light_userdata ptr. number value)");\
        return 1; \
    } \
    luaL_checktype(state, 1, LUA_TLIGHTUSERDATA); \
    lua_Integer integer = luaL_checkinteger(state, 2); \
    if(integer < (MIN) || integer > (MAX)){ \
        luaL_error(state, "exceeds bounds for type %s", #TYPENAME);   \
        return 1;\
    }                              \
    *((TYPENAME*)lua_touserdata(state, 1)) = (TYPENAME) integer;                              \
    return 0; \
    \
}

#define writerfunc_dec(TYPENAME) \
static int lu_write##TYPENAME(lua_State *state) { \
    if(lua_gettop(state) < 2) { \
        luaL_error(state, "not enough arguments (expected light_userdata ptr. number value)");\
        return 1; \
    }                            \
    luaL_checktype(state, 1, LUA_TLIGHTUSERDATA); \
    *((TYPENAME*)lua_touserdata(state, 1)) = (TYPENAME) luaL_checknumber(state, 2);           \
    return 0;\
}


#define readerfunc(TYPENAME) \
static int lu_read##TYPENAME(lua_State *state) { \
    luaL_checktype(state, 1, LUA_TLIGHTUSERDATA); \
    lua_pushinteger(state, (lua_Integer)*(TYPENAME *)(lua_touserdata(state, 1))); \
return 1; \
}

writerfunc(uint8_t, UINT8_MAX, 0)
writerfunc(uint16_t, UINT16_MAX, 0)
writerfunc(uint32_t, UINT32_MAX, 0)
writerfunc(uint64_t, UINT64_MAX, 0)

writerfunc(int16_t, INT16_MAX, INT16_MIN)
writerfunc(int32_t, INT32_MAX, INT32_MIN)
writerfunc(int64_t, INT64_MAX, INT64_MIN)

writerfunc_dec(float)
writerfunc_dec(double)

readerfunc(uint8_t)
readerfunc(uint16_t)
readerfunc(uint32_t)
readerfunc(uint64_t)

readerfunc(int16_t)
readerfunc(int32_t)
readerfunc(int64_t)

readerfunc(float)
readerfunc(double)

static int lu_readstring(lua_State *state) {
    int argcnt = lua_gettop(state);
    luaL_checktype(state, 1, LUA_TLIGHTUSERDATA);
    const char* ptr = (char*)lua_touserdata(state, 1);
    if(argcnt == 1) {
        lua_pushstring(state, ptr);
        return 1;
    }
    if(argcnt == 2) {
        size_t length = luaL_checkinteger(state, 2);
        char tempbuffer[length+1];
        memcpy(tempbuffer, ptr, length);
        tempbuffer[length] = 0;
        lua_pushstring(state,tempbuffer);
        return 1;
    }
    lua_pushnil(state);
    luaL_error(state, "args fucked up");
    return 2;
}
static const luaL_Reg lu_utils_functions[] {
        {"fromhex",    lu_fromhex},
        {"add",        lu_add},
        {"multiply",   lu_multiply},
        {"divide",     lu_divide},
        {"readint64",   lu_readint64_t},
        {"readint32",    lu_readint32_t},
        {"readint16",  lu_readint16_t},
        {"readuint64",   lu_readuint64_t},
        {"readuint32",    lu_readuint32_t},
        {"readuint16",  lu_readuint16_t},
        {"readuint8",   lu_readuint8_t},
        {"readfloat",  lu_readfloat},
        {"readdouble", lu_readdouble},
        {"writeuint8", lu_writeuint8_t},
        {"writeuint16", lu_writeuint16_t},
        {"writeuint32", lu_writeuint32_t},
        {"writeuint64", lu_writeuint64_t},
        {"writeint16", lu_writeint16_t},
        {"writeint32", lu_writeint32_t},
        {"writeint64", lu_writeint64_t},
        {"writefloat", lu_writefloat},
        {"writedouble", lu_writedouble},
        {"readstring", lu_readstring},
        {"readptr", lu_readptr},
        {nullptr,      nullptr}
};
void canvas_api_register(lua_State *state) {
    lua_newtable(state);
    luaL_setfuncs(state, cipher_functions, 0);
    lua_setglobal(state, "cipher");
    lua_newtable(state);
    luaL_setfuncs(state, lu_utils_functions, 0);
    lua_setglobal(state, "lu");
}