#pragma once

#include <app_consts.h>
#include <stdio.h>
#include <log_color.h>

#define LOGS_DEFAULT_TAG "JNK"

template<class LvlType, class TagType, class... Args>
void _app_log_message(LvlType lvl, TagType tag, Args... args) { throw "unexpected realization"; }

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
#define LOGS_LVL_ERROR   COLOR_RED    "Error"
#define LOGS_LVL_WARNING COLOR_YELLOW "Warning"
#define LOGS_LVL_VERBOSE COLOR_CYAN   "Verbose"
#define LOGS_LVL_INFO    COLOR_GREEN  "Info"
#endif

#define PRINT_TAG_E(tag, ...) _app_log_message(LOGS_LVL_ERROR,   tag, __VA_ARGS__)
#if DEBUG_LOGS
#define PRINT_TAG_W(tag, ...) _app_log_message(LOGS_LVL_WARNING, tag, __VA_ARGS__)
#define PRINT_TAG_V(tag, ...) _app_log_message(LOGS_LVL_VERBOSE, tag, __VA_ARGS__)
#define PRINT_TAG_I(tag, ...) _app_log_message(LOGS_LVL_INFO,    tag, __VA_ARGS__)
#else
#define PRINT_TAG_W(tag, ...)
#define PRINT_TAG_V(tag, ...)
#define PRINT_TAG_I(tag, ...)
#endif

#define PRINT_E(...) PRINT_TAG_E(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_W(...) PRINT_TAG_W(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_V(...) PRINT_TAG_V(LOGS_DEFAULT_TAG, __VA_ARGS__)
#define PRINT_I(...) PRINT_TAG_I(LOGS_DEFAULT_TAG, __VA_ARGS__)

#define PRINT(...) PRINT_I(__VA_ARGS__)



#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
template<class... Args>
void _app_log_message(const char* lvl, const char* tag, Args... args) {
    printf("%s  (%s:%d)\t%s\t: ", tag, __FILE__, __LINE__, lvl);
    printf(args...);
    printf("\n" COLOR_RESET); 
}
#else
#error "logs are not supported on this platform"
#endif
