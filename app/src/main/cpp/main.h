//
// Created by Lukas on 01/08/2022.
//

#ifndef USERLIB_MAIN_H
#define USERLIB_MAIN_H

typedef void (*func)();
void Menu();
void Init();

__attribute__ ((visibility ("default")))extern "C"
func Start(){
    Init();
    return &Menu;
}


#endif //USERLIB_MAIN_H
