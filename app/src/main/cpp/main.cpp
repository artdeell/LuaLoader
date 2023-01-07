//
// Created by Lukas on 22/07/2022.
//

#include <unistd.h>
#include "main.h"
#include "includes/cipher/Cipher.h"
#include "includes/imgui/imgui.h"
#include "includes/misc/Logger.h"
#include "lua-include.hpp"
#include "bindings/imgui_lua_bindings.h"
#include "includes/fileselector.h"
#include "canvas_api.h"
#include "cipher_api.h"
#include "dyncall.h"
#include "malloc_search.h"
lua_State *g_state;
char status = 0;
char error_state[4096] = {0};

#define TUMBLE(str) {status = 0; strncpy(error_state, str, 4095);}

static int exit(lua_State *state) {
    status = 0;
    return 0;
}

static void readfcn(int fd) {
    if(fd != -1) {
        lua_State *state = luaL_newstate();
        if(state == nullptr) {
            TUMBLE("Memory allocation error")
            status = 0;
            return;
        }
        luaL_openlibs(state);
        LoadImguiBindings(state);
        canvas_api_register(state);
        cipher_api_register(state);
        dyncall_register(state);
        register_mallocsearch(state);
        lua_pushcfunction(state, exit);
        lua_setglobal(state, "exit");
        char buf[PATH_MAX];
        snprintf(buf, PATH_MAX-1, "/proc/self/fd/%i", fd);
        bool error = luaL_dofile(state, buf); // stack: ???
        close(fd);
        if(error) { // stack: [errorcode]
            snprintf(error_state, 4095, "error during dofile: %s", lua_tostring(state, -1));
            lua_close(state); //stack: fucken gone
            status = 0;
            return;
        }// else, stack: []
        lua_getglobal(state,"draw_func"); //stack: [value of draw func]
        //dumpstack(state);
        if(!lua_isfunction(state, -1) || lua_isnil(state, -1)) {
            TUMBLE("draw_func is not a function");
            lua_close(state); //stack: gone
            status = 0;
            return;
        }
        lua_pop(state, 1);
        g_state = state;
        status = 2;
    }else{
        TUMBLE("No file selected");
        status = 0;
    }
}

static void st_main() {
    if(g_state != nullptr) {
        lua_close(g_state);
        g_state = nullptr;
    }
    ImGui::TextUnformatted("liblualoader.so, made by artDev\nNo, it does not run scripts for GameGuardian");
    if(error_state[0] != 0) ImGui::TextColored(ImVec4(1,0,0,1), "%s", error_state);
    if(ImGui::Button("Load file")) {
        error_state[0] = 0;
        if(requestFile("text/plain", &readfcn, false)) {
            status = 1;
        }
    }
}

static void st_busy() {
    ImGui::TextUnformatted("Wait...");
}

static void st_execscript() {
    lua_getglobal(g_state, "draw_func");
    auto errcode = lua_pcall(g_state, 0, 0, 0);
    if(errcode != 0) {
        snprintf(error_state, 4095, "error during drawfunc: %s", lua_tostring(g_state, -1));
        status = 0;
    }
}

void (*states[])() {
    st_main, st_busy, st_execscript
};

void Menu() {
    states[status]();
}

void Init(){
};