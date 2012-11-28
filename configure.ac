#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# 

AC_INIT(src/main.c)
AM_INIT_AUTOMAKE(mysql_dump_parser,0.4)

# AC_CONFIG_SRCDIR([src/compile_date_time.h])


AM_CONFIG_HEADER(src/config.h)


# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_CHECK_PROG( wc_exist, wc, true)
AC_CHECK_PROG( diff_exist, diff, true)

AS_IF([test  x$wc_exist !=  xtrue -o  x$diff_exist != xtrue ], [
    AC_MSG_WARN([echo "Make check" will not be available. wc and diff are necessary for that. ])
])

AM_CONDITIONAL( CHECK_ENABLED, [test  x$wc_exist =  xtrue -a  x$diff_exist = xtrue   ] )



# Checks for libraries.
AC_SEARCH_LIBS([regcomp], [regex], [], [
  AC_MSG_ERROR([unable to find the regcomp() function])
])

# Checks for header files.
AC_CHECK_HEADERS([regex.h stdlib.h stdio.h string.h unistd.h], [], [
  AC_MSG_ERROR([unable to find the some headers ])
])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([regcomp],,AC_MSG_ERROR([echo Function regcomp is not found]))
AC_CHECK_FUNCS([regfree],,AC_MSG_ERROR([echo Function regfree is not found]))
AC_CHECK_FUNCS([regexec],,AC_MSG_ERROR([echo Function regexec is not found]))
AC_CHECK_FUNCS([regerror],,AC_MSG_ERROR([echo Function regerror is not found]))


AC_PROG_INSTALL

# Output files
AC_CONFIG_FILES([Makefile src/Makefile ])
# AC_CONFIG_FILES([tests/Makefile ])

AC_OUTPUT()
