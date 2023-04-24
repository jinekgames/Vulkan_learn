#pragma once

#include <log_color.h>

#include <stdio.h>

#define LOGS_DEFAULT_TAG "JNK"

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
#define PRINT_LVL_TAG(lvl, tag, ...) { printf("%s  (%s:%d)\t%s\t: ", tag, __FILE__, __LINE__, lvl); printf(__VA_ARGS__); printf("\n" COLOR_RESET); }
#define LOGS_LVL_ERROR   COLOR_RED    "Error"
#define LOGS_LVL_WARNING COLOR_YELLOW "Warning"
#define LOGS_LVL_VERBOSE COLOR_CYAN   "Verbose"
#define LOGS_LVL_INFO    COLOR_GREEN  "Info"
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
