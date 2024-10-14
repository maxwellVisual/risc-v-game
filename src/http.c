#include "http.h"

static void (*http_handler)() = NULL;

void init_http_server(void (*handler)()){
    http_handler = handler;
}
