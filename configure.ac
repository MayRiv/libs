AC_INIT([libs], [1.0])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADERS([autoconfig.h])
AM_INIT_AUTOMAKE()
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_PROG_RANLIB
AC_CANONICAL_HOST

case $host_os in
linux* | Linux*)
        LIB_OS_SPECIFIC_L="-lpthread -lmysqlclient -L/usr/local/lib -L/usr/local/lib/mysql"
        LIB_OS_SPECIFIC_I="-I/usr/include/mysql -I/usr/local/include"
        LIB_OS_SPECIFIC_F="-DLINUX"
        ;;
FreeBSD* | freebsd*)
        AC_CHECK_LIB([bz2], [BZ2_bzBuffToBuffDecompress],[],[
                 echo "BZ2 library is required for this program"
                 exit -1])
        AC_CHECK_LIB([iconv], [iconv_open],[],[
                 echo "iconv library is required for this program"
                 exit -1])
        AC_CHECK_LIB([z], [gzread],[],[
                 echo "ZLib library is required for this program"
                 exit -1])
        AC_CHECK_LIB([m], [round],[],[
                 echo "Math library is required for this program"
                 exit -1])
        LIB_OS_SPECIFIC_L="-lmysqlclient -lz -pthread -lm /usr/local/lib/libiconv.a -L/usr/local/lib/mysql -L/usr/local/lib"
        LIB_OS_SPECIFIC_I="-I/usr/local/include/mysql -I/usr/local/include -I../../libs -I../ -I../indexer -I../resserver -I../spider -I/usr/local/include/gd"
        LIB_OS_SPECIFIC_F="-DBSD"
        ;;
*)
        AC_MSG_ERROR([unsupported operating system])
        ;;
esac

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/statvfs.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_STRUCT_TIMEZONE
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gethostbyname gethostname gettimeofday inet_ntoa isascii localtime_r memchr memmove memset mkdir rmdir setlocale socket strcasecmp strchr strcspn strerror strncasecmp strpbrk strspn strstr strtol strtoul strtoull utime])

AC_SUBST([LIB_OS_SPECIFIC_L])
AC_SUBST([LIB_OS_SPECIFIC_I])
AC_SUBST([LIB_OS_SPECIFIC_F])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
