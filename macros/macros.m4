## GLOM_ARG_ENABLE_WARNINGS()
##
## Provide the --enable-warnings configure argument, set to 'minimum'
## by default.
##
AC_DEFUN([GLOM_ARG_ENABLE_WARNINGS],
[
  AC_ARG_ENABLE([warnings],
      [  --enable-warnings=[[none|minimum|maximum|hardcore]]
                          Control compiler pickyness.  [[default=minimum]]],
      [glom_enable_warnings="$enableval"],
      [glom_enable_warnings='minimum'])

  AC_MSG_CHECKING([for compiler warning flags to use])

  glom_warning_flags=''

  case "$glom_enable_warnings" in
    minimum|yes) glom_warning_flags='-Wall -Wno-long-long';;
    maximum)     glom_warning_flags='-pedantic -W -Wall -Wno-long-long';;
    hardcore)    glom_warning_flags='-pedantic -W -Wall -Wno-long-long -Werror';;
  esac

  glom_use_flags=''

  if test "x$glom_warning_flags" != "x"
  then
    echo 'int foo() { return 0; }' > conftest.cc

    for flag in $glom_warning_flags
    do
      # Test whether the compiler accepts the flag.  GCC doesn't bail
      # out when given an unsupported flag but prints a warning, so
      # check the compiler output instead.
      glom_cxx_out="`$CXX $flag -c conftest.cc 2>&1`"
      rm -f conftest.$OBJEXT
      test "x${glom_cxx_out}" = "x" && \
        glom_use_flags="${glom_use_flags:+$glom_use_flags }$flag"
    done

    rm -f conftest.cc
    glom_cxx_out=''
  fi

  if test "x$glom_use_flags" != "x"
  then
    for flag in $glom_use_flags
    do
      case " $CXXFLAGS " in
        *" $flag "*) ;; # don't add flags twice
        *)           CXXFLAGS="${CXXFLAGS:+$CXXFLAGS }$flag";;
      esac
    done
  else
    glom_use_flags='none'
  fi

  AC_MSG_RESULT([$glom_use_flags])
])
