#include "cmd.h"

#include <stdint.h>

static int handle_opts(size_t argc, const char **argv, Opts **opts){
    const char *arg = argv[0];
    Opts *opt = NULL;
    for (size_t i = 0; opts[i] != NULL; i++)
    {
        if(0 == strcmp(arg, opts[i]->cmd)){
            opt = opts[i];
            break;
        }
    }
    if(opt == NULL){
        return 1;
    }
    if(opt->sub_opts == NULL){
        opt->handler(argc - 1, argv + 1);
        return 0;
    }
    return handle_opts(argc - 1, argv + 1, opt->sub_opts);
}
int parse_opts(char* args, Opts **opts){
    size_t argc = 0;
    size_t i = 0;
    while(1){
        if(args[i] == '\0'){
            break;
        }
        while(args[i] == ' '){
            i++;
        }
        argc++;
        if(args[i] == '\0'){
            break;
        }
        while(args[i] != '\0' && args[i] != ' '){
            i++;
        }
    }
    const char* argv[argc+1];
    i = 0;
    argc = 0;
    while(1){
        if(args[i] == '\0'){
            break;
        }
        while(args[i] == ' '){
            i++;
        }
        argc++;
        if(args[i] == '\0'){
            break;
        }
        argv[argc] = args+i;
        args[i] = '\0';
        i++;
        while(args[i] != '\0' && args[i] != ' '){
            i++;
        }
    }
    argv[argc] = NULL;

    return handle_opts(argc, argv, opts);
}

