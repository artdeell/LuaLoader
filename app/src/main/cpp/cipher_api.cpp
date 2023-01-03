//
// Created by maks on 03.01.2023.
//

#include <android/log.h>
#include "cipher_api.h"
#include "lua-include.hpp"
#include "includes/cipher/Cipher.h"
static int cipherpatch_new(lua_State *state) {
    new ((CipherPatch*)lua_newuserdata(state, sizeof(CipherPatch))) CipherPatch();
    luaL_getmetatable(state, "CipherPatch");
    lua_setmetatable(state, -2);
    return 1;
}
static int cipherpatch_set_opcode(lua_State *state) {
    auto *patch = (CipherPatch*)luaL_checkudata(state, 1, "CipherPatch");
    const char* opc_hex = luaL_checkstring(state, 2);
    patch->set_Opcode(std::string(opc_hex));
    return 0;
}
static int cipherbase_fire(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    __android_log_print(ANDROID_LOG_INFO, "luaexec", "base->Fire()");
    base->Fire();
    return 0;
}
static int cipherbase_restore(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    base->Restore();
    return 0;
}
static int cipherbase_destructor(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    base->CipherBase::~CipherBase();
    return 0;
}
static int cipherbase_set_libname(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    const char* libname = luaL_checkstring(state, 2);
    base->set_libName(libname);
    return 0;
}
static int cipherbase_set_lock(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    luaL_checktype(state, 2, LUA_TBOOLEAN);
    bool lock = lua_toboolean(state, 2);
    base->set_Lock(lock);
    return 0;
}

static void dumpstack (lua_State *L) {
    int top=lua_gettop(L);
    for (int i=1; i <= top; i++) {
        __android_log_print(ANDROID_LOG_INFO, "luaexec", "%d\t%s\t", i, luaL_typename(L,i));
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                __android_log_print(ANDROID_LOG_INFO, "luaexec", "%g\n",lua_tonumber(L,i));
                break;
            case LUA_TSTRING:
                __android_log_print(ANDROID_LOG_INFO, "luaexec", "%s\n",lua_tostring(L,i));
                break;
            case LUA_TBOOLEAN:
                __android_log_print(ANDROID_LOG_INFO, "luaexec", "%s\n", (lua_toboolean(L, i) ? "true" : "false"));
                break;
            case LUA_TNIL:
                __android_log_print(ANDROID_LOG_INFO, "luaexec", "%s\n", "nil");
                break;
            default:
                __android_log_print(ANDROID_LOG_INFO, "luaexec", "%p\n",lua_topointer(L,i));
                break;
        }
    }
}

static int cipherbase_set_address(lua_State *state) {
    auto *base = (CipherBase*)luaL_testudata(state, 1, "CipherPatch");
    if(base == nullptr) base = (CipherBase*) luaL_testudata(state, 1, "CipherHook");
    if(base == nullptr) {
        lua_pushstring(state, "Not a CipherPatch or a CipherHook");
        lua_error(state);
        return 1;
    }
    int argcnt = lua_gettop(state);
    __android_log_print(ANDROID_LOG_INFO, "luaexec", "argcnt: %i", argcnt);
    if(argcnt == 2) {
        auto type = lua_type(state,2);
        if(type == LUA_TLIGHTUSERDATA) {
            auto address = (uintptr_t)lua_touserdata(state, 2);
            base->set_Address(address);
            return 0;
        }
        if (type == LUA_TSTRING) {
            const char* symbol = lua_tostring(state, 2);
            base->set_Address(symbol);
            return 0;
        }
    }
    if(argcnt == 3) {
        auto type = lua_type(state,2);
        auto type2 = lua_type(state, 3);
        __android_log_print(ANDROID_LOG_INFO, "luaexec", "cond 2, types: %s %s %d %d", lua_typename(state, type), lua_typename(state, type2), type == LUA_TUSERDATA, type2 == LUA_TBOOLEAN);
        if(type == LUA_TLIGHTUSERDATA && type2 == LUA_TBOOLEAN) {
            __android_log_print(ANDROID_LOG_INFO, "luaexec", "cond 2.1");
            auto address = (uintptr_t)lua_touserdata(state, 2);
            bool isLocal = lua_toboolean(state, 3);
            base->set_Address(address, isLocal);
            __android_log_print(ANDROID_LOG_INFO, "luaexec", "cond 2.1 returning");
            return 0;
        }
        if(type == LUA_TSTRING && type2 == LUA_TSTRING) {
            const char* pattern = lua_tostring(state, 2);
            const char* mask = lua_tostring(state, 3);
            base->set_Address(pattern, mask);
            return 0;
        }
    }
    dumpstack(state);
    lua_pushstring(state, "Looks like ye fucked up with the types.");
    lua_error(state);
    return 1;
}

static const luaL_Reg cipherpatch_f[] {
        {"new", cipherpatch_new},
        {nullptr, nullptr}
};
static const luaL_Reg cipherpatch_m[] {
        {"set_Opcode", cipherpatch_set_opcode},
        {"Fire", cipherbase_fire},
        {"Restore", cipherbase_restore},
        {"set_libName", cipherbase_set_libname},
        {"set_Lock", cipherbase_set_lock},
        {"set_Address", cipherbase_set_address},
        {"__gc", cipherbase_destructor},
        {nullptr, nullptr}
};

void cipher_api_register(lua_State *state) {
    luaL_newmetatable(state, "CipherPatch");
    lua_pushstring(state, "__index");
    lua_pushvalue(state, -2);  /* pushes the metatable */
    lua_settable(state, -3);  /* metatable.__index = metatable */
    luaL_setfuncs(state, cipherpatch_m, 0);
    lua_newtable(state);
    luaL_setfuncs(state, cipherpatch_f, 0);
    lua_setglobal(state, "cipherpatch");
}