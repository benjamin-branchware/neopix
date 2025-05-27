AC_PREREQ([2.69])
AC_INIT([neopix], [0.1], [crawford.benjamin15@gmail.com])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

PKG_CHECK_MODULES([GTK4], [gtk4 >= 4.10])
# Find Cairo with SVG backend
PKG_CHECK_MODULES([CAIRO], [cairo >= 1.0 cairo-svg >= 1.0])

AC_SUBST([GTK4_CFLAGS])
AC_SUBST([GTK4_LIBS])
AC_SUBST([CAIRO_CFLAGS])
AC_SUBST([CAIRO_LIBS])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
