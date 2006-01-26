dnl Copyright (c) 2002  Leon Bottou and Yann Le Cun.
dnl Copyright (c) 2001  AT&T
dnl
dnl Most of these macros are derived from macros listed
dnl at the GNU Autoconf Macro Archive
dnl http://www.gnu.org/software/ac-archive/
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111 USA
dnl

dnl -------------------------------------------------------
dnl @synopsis AC_VARIFY(varname)
dnl Replace expansion of $libdir, $datadir, $bindir, $prefix
dnl by references to the variable.
dnl -------------------------------------------------------
AC_DEFUN([AC_VARIFY],[
    xdir="`eval echo \"$libdir\"`"
    $1=`echo [$]$1 | sed -e 's:^'"$xdir"'/:${libdir}/:'`
    xdir="`eval echo \"$datadir\"`"
    $1=`echo [$]$1 | sed -e 's:^'"$xdir"'/:${datadir}/:'`
    xdir="`eval echo \"$bindir\"`"
    $1=`echo [$]$1 | sed -e 's:^'"$xdir"'/:${bindir}/:'`
    xdir="`eval echo \"$prefix\"`"
    $1=`echo [$]$1 | sed -e 's:^'"$xdir"'/:${prefix}/:'`
])


dnl -------------------------------------------------------
dnl @synopsis AC_LOCATE_DESKTOP_DIRS
dnl Define installation paths for desktop config files
dnl (mime types, menu entries, icons, etc.)
dnl -------------------------------------------------------
AC_DEFUN([AC_FIND_DESKTOP_DIRS],[
   if test "${prefix}" == "/usr" ; then
     ac_desktopfiles=yes
   else
     ac_desktopfiles=no
   fi
   AC_ARG_ENABLE(desktopfiles,
        AC_HELP_STRING([--enable-desktopfiles],
            [Install icons and menu files (default: only when prefix=/usr).]),
        [ac_desktopfiles=$enableval])

   dtop_applications=          # XDG menu entries
   dtop_icons=                 # KDE-style icon directories
   dtop_pixmaps=               # Gnome-style icon directories
   dtop_mime_info=             # Gnome mime database
   dtop_application_registry=  # Gnome mime associations
   dtop_applnk=                # KDE menu entries (KDE<3.2)
   dtop_mimelnk=               # KDE mime database
   dtop_menu=                  # DEBIAN menus

   if test $ac_desktopfiles != no 
   then
       # kde-config
       KDE_CONFIG=
       AC_PATH_PROG(KDE_CONFIG, kde-config)

       # dtop_menu
       AC_MSG_CHECKING([for Debian menu directory])
       if test -d /etc/menu-methods -a -d /usr/share/menu ; then
           dtop_menu="/usr/share/menu"
       elif test -d /etc/menu-methods -a -d /usr/lib/menu ; then
           dtop_menu="/usr/lib/menu"
       fi
       AC_VARIFY(dtop_menu)
       AC_MSG_RESULT(${dtop_menu:-no})
   
       # dtop_applications
       AC_MSG_CHECKING([for XDG menu directory])
       if test -d /usr/share/applications ; then
           if test -r /etc/xdg/menus/applications.menu ||
              test -r /etc/X11/desktop-menus/applications.menu ; then
                dtop_applications="/usr/share/applications"
           fi
       fi
       AC_VARIFY(dtop_applications)
       AC_MSG_RESULT(${dtop_applications:-no})

       # dtop_pixmaps
       AC_MSG_CHECKING([for Gnome icon directory])
       if test -d "/usr/share/pixmaps" ; then
           dtop_pixmaps="/usr/share/pixmaps"
       fi
       AC_VARIFY(dtop_pixmaps)
       AC_MSG_RESULT(${dtop_pixmaps:-no})

       # dtop_mime_info
       AC_MSG_CHECKING([for Gnome mimetype directory])
       if test -d "/usr/share/mime-info" ; then
           dtop_mime_info="/usr/share/mime-info"
       fi
       AC_VARIFY(dtop_mime_info)
       AC_MSG_RESULT(${dtop_mime_info:-no})

       # dtop_application_registry
       AC_MSG_CHECKING([for Gnome association directory])
       if test -d "/usr/share/application-registry" ; then
           dtop_application_registry="/usr/share/application-registry"
       fi
       AC_VARIFY(dtop_application_registry)
       AC_MSG_RESULT(${dtop_application_registry:-no})

       # dtop_icons
       AC_MSG_CHECKING([for KDE icon directory])
       if test -x "$KDE_CONFIG" ; then
           dtop_icons=`$KDE_CONFIG --expandvars --install icon`
       elif test -d "/usr/share/icons" ; then
           dtop_icons="/usr/share/icons"
       fi
       AC_VARIFY(dtop_icons)
       AC_MSG_RESULT(${dtop_icons:-no})

       # dtop_applnk
       if test -z "$dtop_menu" -a -z "$dtop_applications" ; then
           AC_MSG_CHECKING([for KDE menu directory])
           if test -x "$KDE_CONFIG" ; then
               dtop_applnk=`$KDE_CONFIG --expandvars --install apps`
           elif test -d "/usr/share/applnk" ; then
               dtop_applnk="/usr/share/applnk"
           fi
           AC_VARIFY(dtop_applnk)
           AC_MSG_RESULT(${dtop_applnk:-no})
       fi

       # dtop_mimelnk
       AC_MSG_CHECKING([for KDE mimetype directory])
       if test -x "$KDE_CONFIG" ; then
           dtop_mimelnk=`$KDE_CONFIG --expandvars --install mime`
       elif test -d "/usr/share/mimelnk" ; then
           dtop_mimelnk="/usr/share/mimelnk"
       fi
       AC_VARIFY(dtop_mimelnk)
       AC_MSG_RESULT(${dtop_mimelnk:-no})
   fi

   AC_SUBST(dtop_applications)
   AC_SUBST(dtop_icons)
   AC_SUBST(dtop_pixmaps)
   AC_SUBST(dtop_mime_info)
   AC_SUBST(dtop_application_registry)
   AC_SUBST(dtop_applnk)
   AC_SUBST(dtop_mimelnk)
   AC_SUBST(dtop_menu)
])


