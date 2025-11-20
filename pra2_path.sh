
if ( $?LD_LIBRARY_PATH ) then
    setenv LD_LIBRARY_PATH /usr/lib/swipl/lib/x86_64-linux/:$LD_LIBRARY_PATH
else
    setenv LD_LIBRARY_PATH /usr/lib/swipl/lib/x86_64-linux/
endif
