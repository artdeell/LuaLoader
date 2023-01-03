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
    if(lua_isuserdata(state, 1)) {
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
    if(lua_isuserdata(state, 1)) {
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

static const luaL_Reg lu_utils_functions[] {
        {"fromhex", lu_fromhex},
        {"add", lu_add},
        {"multiply", lu_multiply},
        {"divide", lu_divide},
        {nullptr, nullptr}
};
void canvas_api_register(lua_State *state) {
    lua_newtable(state);
    luaL_setfuncs(state, cipher_functions, 0);
    lua_setglobal(state, "cipher");
    lua_newtable(state);
    luaL_setfuncs(state, lu_utils_functions, 0);
    lua_setglobal(state, "lu");
}