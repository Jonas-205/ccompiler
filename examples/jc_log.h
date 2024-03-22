/* jcl - v1.0.0 - Fancy logging in C

Do this:
    #define JC_LOG_IMPLEMENTATION
before you include this file in *one* C or C++ file to create the implementation.

// i.e. it should look like this:
#include ...
#include ...
#include ...
#define JC_LOG_IMPLEMENTATION
#include "jcl.h"

QUICK NOTES:
    - I mostly try to follow the stb-style in this library
        (https://github.com/nothings/stb/blob/master/docs/stb_howto.txt)

THINGS YOU CAN DEFINE:
DEFINE EVERYWHERE:
    - JC_LOG_STATIC             make all functions static, only use this if you know what you are doing
    - JC_LOG_DISABLE            disables all logging (die still works and prints only the message to stderr)
    - JC_LOG_DISABLE_ASSERTS    disables jlog_assert
    - JC_LOG_NO_SHORT_NAMES     removes short names (trace, info, ...)

  DEFINE IN SAME FILE AS JLOG_IMPLEMENTATION:
    - JC_LOG_STDOUT         some logging functions use stdout (trace, info), you can override this here
    - JC_LOG_STDERR         some logging functions use stderr (warn, error, die), you can override this here
    - JC_LOG_DISABLE_COLORS     disables colored logging

LICENSE:
    See end of file for license information

*/


#ifndef JC_LOG_H_
#define JC_LOG_H_

#ifdef __cplusplus
extern "C" {
#define noreturn [[noreturn]]
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdnoreturn.h>

#if defined(_WIN32) || defined(WIN32)
#define JC_LOG_WINDOWS
#elif __APPLE__
#define JC_LOG_APPLE
#endif

#ifndef JC_LOG_DEF
#ifdef JC_LOG_STATIC
#define JC_LOG_DEF static
#else
#define JC_LOG_DEF extern
#endif
#endif

#ifndef JC_LOG_STDOUT
#define JC_LOG_STDOUT stdout
#endif
#ifndef JC_LOG_STDERR
#define JC_LOG_STDERR stderr
#endif

#ifdef JC_LOG_WINDOWS
#define jcl_force_inline static __forceinline
#else
#define jcl_force_inline static __attribute__((always_inline)) inline
#endif

// TODO(me): Once C23 comes out, check for it and use its feature if possible
#ifndef jcl_warn_unused
#ifdef JC_LOG_WINDOWS  // afaik windows does not support checking for unused results
#define jcl_warn_unused
#else
#define jcl_warn_unused __attribute__((warn_unused_result))
#endif
#endif

// Colored Io

typedef enum {
    JCL_COLOR_RESET   = 0,
    JCL_COLOR_BLACK   = 30,
    JCL_COLOR_RED     = 31,
    JCL_COLOR_GREEN   = 32,
    JCL_COLOR_YELLOW  = 33,
    JCL_COLOR_BLUE    = 34,
    JCL_COLOR_MAGENTA = 35,
    JCL_COLOR_CYAN    = 36,
    JCL_COLOR_WHITE   = 37,
    JCL_COLOR_DEFAULT = 39,
    JCL_COLOR_GRAY           = 90,
    JCL_COLOR_BRIGHT_RED     = 91,
    JCL_COLOR_BRIGHT_GREEN   = 92,
    JCL_COLOR_BRIGHT_YELLOW  = 93,
    JCL_COLOR_BRIGHT_BLUE    = 94,
    JCL_COLOR_BRIGHT_MAGENTA = 95,
    JCL_COLOR_BRIGHT_CYAN    = 96,
    JCL_COLOR_BRIGHT_WHITE   = 97,
} JclColor;

typedef enum {
    JCL_COLOR_ATTR_RESET      = 0,
    JCL_COLOR_ATTR_BRIGHT     = 1,
    JCL_COLOR_ATTR_DIM        = 2,
    JCL_COLOR_ATTR_UNDERSCORE = 4,
    JCL_COLOR_ATTR_BLINK      = 5,
    JCL_COLOR_ATTR_REVERSE    = 7,
    JCL_COLOR_ATTR_HIDDEN     = 8,
    JCL_COLOR_ATTR_STRIKE     = 9,
    JCL_COLOR_ATTR_DOUBLE_UNDERSCORE = 21,
} JclColorAttribute;

// Colored io functions
jcl_force_inline int jcl_color_attribute(FILE *out, JclColorAttribute attr) {
    return fprintf(out, "\033[%dm", attr);
}
jcl_force_inline int jcl_color_fg(FILE *out, JclColor color) {
    return fprintf(out, "\033[%dm", color);
}
jcl_force_inline int jcl_color_bg(FILE *out, JclColor color) {
    return fprintf(out, "\033[%dm", color + 10);
}
jcl_force_inline int jcl_color_set(FILE *out, JclColor fg, JclColor bg, JclColorAttribute attr) {
    return fprintf(out, "\033[%d;%d;%dm", attr, fg, bg + 10);
}
jcl_force_inline int jcl_color_reset(FILE *out) {
    return fprintf(out, "\033[0m");
}

// Logging


// Levels
typedef enum {
    JCL_LEVEL_TRACE  = 0,
    JCL_LEVEL_INFO   = 1,
    JCL_LEVEL_WARN   = 2,
    JCL_LEVEL_ERROR  = 3,
    JCL_LEVEL_DIE    = 4,
    JCL_LEVEL_ASSERT = 5,
} JclLevel;

// Internal Logging functions
JC_LOG_DEF int jcli__log(const char *file, uint32_t line, JclLevel level, bool newline, const char *format, ...);
noreturn JC_LOG_DEF void jcli__die(void);

// Log related functions
JC_LOG_DEF void jcl_level(JclLevel level);
JC_LOG_DEF JclLevel jcl_level_current(void);
JC_LOG_DEF void jcl_toggle(bool active);
JC_LOG_DEF bool jcl_active(void);
JC_LOG_DEF const char *jcl_level_str(JclLevel level);

#if !defined(JC_DISABLE_LOGGING) || !defined(JC_DISABLE_ASSERTS)
JC_LOG_DEF jcl_warn_unused bool jcl_file(const char *file);  // NULL will disable file logging; check errno on fail
#else
jcl_warn_unused jcl_force_inline bool jcl_file(const char *file) {
    return true;
}
#endif
jcl_warn_unused jcl_force_inline bool jcl_file_close(void) {  // die / assert call this themself; check errno on fail
    return jcl_file(NULL);
}

#ifndef jcl_die
// jcl_die uses printf internally (and also closes the logfile),
// if you want to call this because printf / fclose failed,
// DO NOT DO THAT!!!!
#define jcl_die(...) do { \
    jcli__log(__FILE__, __LINE__, JCL_LEVEL_DIE, true, __VA_ARGS__); \
    jcli__die(); \
} while (0)
#endif

#ifndef JC_DISABLE_LOGGING
#ifndef jcl_trace
#define jcl_trace(...) jcli__log(__FILE__, __LINE__, JCL_LEVEL_TRACE, true, __VA_ARGS__)
#endif

#ifndef jcl_info
#define jcl_info(...) jcli__log(__FILE__, __LINE__, JCL_LEVEL_INFO, true, __VA_ARGS__)
#endif

#ifndef jcl_warn
#define jcl_warn(...) jcli__log(__FILE__, __LINE__, JCL_LEVEL_WARN, true, __VA_ARGS__)
#endif

#ifndef jcl_error
#define jcl_error(...) jcli__log(__FILE__, __LINE__, JCL_LEVEL_ERROR, true, __VA_ARGS__)
#endif

#ifndef jcl_prefix
#define jcl_prefix(level) jcli__log(__FILE__, __LINE__, level, false, "")
#endif

#else  // JC_DISABLE_LOGGING
#undef jcl_trace
#define jcl_trace(...)

#undef jcl_info
#define jcl_info(...)

#undef jcl_warn
#define jcl_warn(...)

#undef jcl_error
#define jcl_error(...)

#undef jcl_prefix
#define jcl_prefix(level)

#endif  // JC_DISABLE_LOGGING

#ifndef JC_DISABLE_ASSERTS
#ifndef jcl_assert
#define jcl_assert(condition, ...) \
    do { if (!(condition)) { \
        jcli__log(__FILE__, __LINE__, JCL_LEVEL_ASSERT, true, #condition); \
        jcli__log(__FILE__, __LINE__, JCL_LEVEL_ASSERT, true, __VA_ARGS__); \
        jcli__die(); \
    }} while (0)
#endif
#else  // JC_DISABLE_ASSERTS
#undef jcl_assert
#define jcl_assert(condition, ...)
#endif  // JC_DISABLE_ASSERTS

#ifndef JC_LOG_NO_SHORT_NAMES
#define trace(...) jcl_trace(__VA_ARGS__)
#define info(...) jcl_info(__VA_ARGS__)
#define warn(...) jcl_warn(__VA_ARGS__)
#define error(...) jcl_error(__VA_ARGS__)
#define die(...) jcl_die(__VA_ARGS__)
#endif  // JC_LOG_NO_SHORT_NAMES





#ifdef __cplusplus
#undef noreturn
}
#endif
//
//
////   end header file   /////////////////////////////////////////////////////
#endif  // JC_LOG_H_

#ifdef JC_LOG_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#define noreturn [[noreturn]]
#endif

#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static FILE *JCI__LOG_FILE_PTR = NULL;
static JclLevel JCI__LOG_LEVEL = JCL_LEVEL_TRACE;
static bool JCI__LOGGING = true;

#if !defined(JC_DISABLE_LOGGING) || !defined(JC_DISABLE_ASSERTS)
JC_LOG_DEF jcl_warn_unused bool jcl_file(const char *file) {
    if (JCI__LOG_FILE_PTR != NULL) {
        int err = fclose(JCI__LOG_FILE_PTR);
        JCI__LOG_FILE_PTR = NULL;
        if (err) {
            return false;
        }
    }
    if (file == NULL) {
        return true;
    }
    JCI__LOG_FILE_PTR = fopen(file, "w");
    return JCI__LOG_FILE_PTR != NULL;
}
#endif

JC_LOG_DEF const char *jcl_level_str(JclLevel level) {
    switch (level) {
        case JCL_LEVEL_TRACE:
            return "trace";
        case JCL_LEVEL_INFO:
            return "info";
        case JCL_LEVEL_WARN:
            return "warn";
        case JCL_LEVEL_ERROR:
            return "error";
        case JCL_LEVEL_DIE:
            return "die";
        case JCL_LEVEL_ASSERT:
            return "assert";
        default:
            return "unknown";
    }
}

JC_LOG_DEF void jcl_level(JclLevel level) {
    JCI__LOG_LEVEL = level;
}

JC_LOG_DEF JclLevel jcl_level_current(void) {
    return JCI__LOG_LEVEL;
}

JC_LOG_DEF void jcl_toggle(bool active) {
    JCI__LOGGING = active;
}

JC_LOG_DEF bool jcl_active(void) {
    return JCI__LOGGING;
}

noreturn JC_LOG_DEF void jcli__die(void) {
    int ignore = jcl_file_close();
    (void) ignore;  // if closing the log file fails here there is not much we can do
#ifdef JC_LOG_WINDOWS
    __debugbreak();
#else
    __builtin_trap();
#endif
}


#define JCI__PRINTF(fun) do { \
    int jcil_helper = fun; \
    if (jcil_helper < 0) { \
        return jcil_helper; \
    } \
    sum += jcil_helper; \
} while (0)



#ifdef JC_DISABLE_LOGGING_COLORS
#define JCI__SET_LOGGING_COLOR(out, color)
#define JCI__RESET_LOGGING_COLOR(out)
#else  // JC_DISABLE_LOGGING_COLORS
#define JCI__SET_LOGGING_COLOR(out, color) JCI__PRINTF(jcl_color_fg(out, color))
#define JCI__RESET_LOGGING_COLOR(out) JCI__PRINTF(jcl_color_reset(out))
#endif  // JC_DISABLE_LOGGING_COLORS


JC_LOG_DEF int jcli__log(const char *file, uint32_t line, JclLevel level, bool newline, const char *format, ...) {
#ifdef JC_DISABLE_LOGGING
    (void) file;
    (void) line;
    (void) newline;
    if (level < JCL_LEVEL_DIE) {
        return 0;
    }
    va_list args;
    va_start(args, format);
    // this is only called on die and writes to stderr, return value kind of does not matter
    int written = vfprintf(JC_LOG_STDERR, format, args);
    va_end(args);

    if (JCI__LOG_FILE_PTR) {
        va_list args;
        va_start(args, format);
        vfprintf(JCI__LOG_FILE_PTR, format, args);  // this is only called if library logging is off and die is called
        va_end(args);
    }

    return written;
#else  // JC_DISABLE_LOGGING
    if ((level < JCI__LOG_LEVEL || !JCI__LOGGING) && (level < JCL_LEVEL_DIE)) {
        return 0;
    }
    FILE *out = JC_LOG_STDERR;
    if (level == JCL_LEVEL_INFO || level == JCL_LEVEL_TRACE) {
        out = JC_LOG_STDOUT;
    }

    int sum = 0;

    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);

    char time_str[9];
    strftime(time_str, 9, "%H:%M:%S", timeinfo);  // never fails if buffer is large enough

    JCI__RESET_LOGGING_COLOR(out);
    JCI__SET_LOGGING_COLOR(out, JCL_COLOR_GRAY);
    JCI__PRINTF(fprintf(out, "%s ", time_str));

    switch (level) {
        case JCL_LEVEL_TRACE:
            JCI__SET_LOGGING_COLOR(out, JCL_COLOR_DEFAULT);
            break;
        case JCL_LEVEL_INFO:
            JCI__SET_LOGGING_COLOR(out, JCL_COLOR_BRIGHT_BLUE);
            break;
        case JCL_LEVEL_WARN:
            JCI__SET_LOGGING_COLOR(out, JCL_COLOR_BRIGHT_YELLOW);
            break;
        case JCL_LEVEL_ERROR:
        case JCL_LEVEL_DIE:
        case JCL_LEVEL_ASSERT:
            JCI__SET_LOGGING_COLOR(out, JCL_COLOR_RED);
            break;
        default:
            break;
    }
    JCI__PRINTF(fprintf(out, "%s\t", jcl_level_str(level)));

#ifndef JC_LOG_DISABLE_FILE_LOGGING
    JCI__SET_LOGGING_COLOR(out, JCL_COLOR_GRAY);
    JCI__PRINTF(fprintf(out, "%s:%u\t", file, line));
#endif  // JC_LOG_DISABLE_FILE_LOGGING
    JCI__RESET_LOGGING_COLOR(out);
    va_list args;
    va_start(args, format);
    JCI__PRINTF(vfprintf(out, format, args));
    va_end(args);
    if (newline) {
        JCI__PRINTF(fprintf(out, "\n"));
    }

    int erg = sum;

    if (JCI__LOG_FILE_PTR) {
        JCI__PRINTF(fprintf(JCI__LOG_FILE_PTR, "%s %s\t%s:%u\t", time_str, jcl_level_str(level), file, line));
        va_list args;
        va_start(args, format);
        JCI__PRINTF(vfprintf(JCI__LOG_FILE_PTR, format, args));
        va_end(args);
        if (newline) {
            JCI__PRINTF(fprintf(JCI__LOG_FILE_PTR, "\n"));
        }
    }

    return erg;
#endif  // JC_DISABLE_LOGGING
}

#ifdef __cplusplus
#undef noreturn
}
#endif
#endif  // JC_LOG_IMPLEMENTATION

/*
------------------------------------------------------------------------------
MIT License

Copyright (c) 2023 Jonas Kristen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
*/
