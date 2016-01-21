

cc -I"/usr/include/pd" -I"../../common" -Wall -W -g -DPD -DVERSION='"0.2"' -fPIC -O6 -funroll-loops -fomit-frame-pointer -o "bytes2any.o" -c "bytes2any.c"

cc  -Wl,--export-dynamic  -shared -fPIC -o "bytes2any.pd_linux" "bytes2any.o"  -lc


