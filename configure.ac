#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.72])
AC_INIT([cleanimg], [1.1], [github.com/ottavm/cleanimg/issues], [], [github.com/ottavm/cleanimg])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdint.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate memmove memset munmap setlocale strdup])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
