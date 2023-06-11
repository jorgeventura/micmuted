#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([micmuted], [0.1.0], [jorge.araujo.ventura@gmail.com])
AC_CONFIG_SRCDIR([micmuted.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([pulse],[pa_mainloop_new],[],[AC_MSG_FAILURE("Install libpulse package")],[])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
