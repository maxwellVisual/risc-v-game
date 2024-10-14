#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>

__attribute__((weak)) void error_handler(void){
    exit(-1);
}
__attribute__((weak)) void exit_handler(int status, void *args){
    exit(status);
}
__attribute__((weak)) void signal_handler(int signal){
    exit(1);
}
extern void init_error_handler(void){
    if(0 != on_exit(exit_handler, NULL)){
        error_handler();
    }
    const int signals[] = {
        SIGHUP,
        SIGINT,
        SIGTERM
    };
    for(uint8_t i=0; i<(sizeof(signals)/sizeof(*signals)); i++){
        (void)signal(signals[i], signal_handler);
    }
}

