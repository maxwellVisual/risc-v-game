#pragma once

#ifdef CONFIG_DYNAMIC_ERROR_LOG
#define ERROR_LOG(...) (void)fprintf(stderr, __VA_ARGS__)
#else
#define ERROR_LOG(...)
#endif

/**
 * @attention this function should always exit the program or correct the error
 */
extern void error_handler(void);
extern void exit_handler(int status, void *args);
extern void init_error_handler(void);
