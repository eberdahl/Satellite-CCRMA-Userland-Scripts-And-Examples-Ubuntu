cc -I"/usr/include/pd" -Wall -W -g -DPD -DVERSION='"0.2"' -fPIC -O6 -funroll-loops -fomit-frame-pointer -o "shell.o" -c "shell.c"

cc  -Wl,--export-dynamic  -shared -fPIC -o "shell.pd_linux" "shell.o"   -lc

chmod a-x "shell.pd_linux"

