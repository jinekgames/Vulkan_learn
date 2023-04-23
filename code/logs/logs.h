#pragma once

#include <stdio.h>

#define LOGS_DEFAULT_TAG "JNK"

#if defined(WIN32)
#define PRINT_LVL_TAG(lvl, tag, ...) { printf("%s  (%s:%d)\t%s : ", tag, __FILE__, __LINE__, lvl); printf(__VA_ARGS__); printf("\n"); }
#define LOGS_LVL_ERROR   "Error"
#define LOGS_LVL_WARNING "Warning"
#define LOGS_LVL_VERBOSE "Verbose"
#define LOGS_LVL_INFO    "Info"
#else
#error "logs unsupported"
#endif

#define PRINT_TAG_E(tag, ...) PRINT_LVL_TAG(LOGS_LVL_ERROR,   tag, __VA_ARGS__)
#define PRINT_TAG_W(tag, ...) PRINT_LVL_TAG(LOGS_LVL_WARNING, tag, __VA_ARGS__)
#define PRINT_TAG_V(tag, ...) PRINT_LVL_TAG(LOGS_LVL_VERBOSE, tag, __VA_ARGS__)
#define PRINT_TAG_I(tag, ...) PRINT_LVL_TAG(LOGS_LVL_INFO,    tag, __VA_ARGS__)

#define PRINT_E(...) PRINT_TAG_E(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_W(...) PRINT_TAG_W(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_V(...) PRINT_TAG_V(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_I(...) PRINT_TAG_I(LOGS_DEFAULT_TAG, __VA_ARGS__)

#define PRINT(...) PRINT_I(__VA_ARGS__)
