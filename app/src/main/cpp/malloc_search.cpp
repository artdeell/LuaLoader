//
// Created by maks on 06.01.2023.
//

#include <cstdlib>
#include <cstring>
#include <android/log.h>
#include "malloc_search.h"

const char* scudo_search_p = "[anon:scudo:";
const size_t scudo_search_s = strlen(scudo_search_p);
const char* libc_search_p = "[anon:libc_malloc";
const size_t libc_search_s = strlen(libc_search_p);

static void* find(const uintptr_t pstart, const size_t size, const char *pattern, const char *mask)
{
    size_t howMuch = strlen(mask);
    char* start = (char*)pstart;
    if(size < howMuch) return nullptr;
    for(size_t i = 0; i < size; i++) {
        for(size_t j = 0; j < howMuch; j++) {
            if(i+j >= size) return nullptr;
            if(start[i+j] != pattern[j] && mask[j] == 'x') break;
            if(j == howMuch - 1) return &start[i];
        }
    }
    return nullptr;
}

static int malloc_search(lua_State *state) {
    const char* pattern = luaL_checkstring(state, 1);
    const char* mask = luaL_checkstring(state, 2);

    char* line = nullptr;
    size_t len = 0;
    FILE* file = fopen("/proc/self/maps", "r");
    if(file == nullptr) {
        lua_pushnil(state);
        lua_pushstring(state, "Failed to read procmaps");
        lua_error(state);
        return 2;
    }
    bool scudoPageFound = false;
    while (getline(&line, &len, file) != -1) {
        char* searchstart = strchr(line, '[');
        if(searchstart == nullptr) continue;
        if(!strncmp(searchstart,scudo_search_p, scudo_search_s) || !strncmp(searchstart,libc_search_p, libc_search_s)) {
            char*end0 = nullptr;
            unsigned long addr0 = strtoul(line, &end0, 0x10);
            char*end1 = nullptr;
            unsigned long addr1 = strtoul(end0+1, &end1, 0x10);
            void* found = find(addr0, addr1-addr0-1, pattern, mask);
            if(found) {
                free(line);
                fclose(file);
                lua_pushlightuserdata(state, found);
                return 1;
            }
            scudoPageFound = true;
        }
    }
    free(line);
    fclose(file);
    if(!scudoPageFound) {
        lua_pushnil(state);
        lua_pushstring(state, "No scudo pages");
        lua_error(state);
        return 2;
    }else{
        lua_pushnil(state);
        return 1;
    }
}

void register_mallocsearch(lua_State *state) {
    lua_pushcfunction(state, malloc_search);
    lua_setglobal(state, "malloc_search");
}