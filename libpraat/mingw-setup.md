# Mingw Setup for Ubuntu 14.04 LTS

The following packages are required:

```sudo apt-get install build-essential mingw-64```

You will also need the 64-bit version of the `GdiPlus.dll` file from a 64-bit version of Windows.

## Setup mingw64

```
sudo ln -s /usr/x86_64-w64-mingw32 /mingw64
cd /mingw66
sudo mkdir bin
cd bin


for f in /usr/bin/x86_64-w64-mingw32-*; do 
    sudo ln -s $f
done


for f in *; do 
    name=`echo $f | awk -F '-' '{print $4}'`
    sudo ln -s $f $name
done

cd ../include
sudo ln -s shlobj.h Shlobj.h
sudo ln -s gdiplus.h GdiPlus.h
sudo cp /path/to/GdiPlus64bit.dll /mingw64/lib
```

> The setup of mingw32 is simliar.  Create the symlink `/mingw32` linking to `/usr/i686-w64-mingw32` and repeat the instructions above for the 32-bit version of mingw.

## Fix Macros

If you get errors referencing the macro `__buildstos` or the file `/mingw64/include/psdk_inc/intrin-impl.h` you may need to make changes to the file `intrin-impl.h`.

Lines 78-84: Change `Data` to `DataXXX`

```
78 #define __buildstos(x, y, z) void x(y *Dest, y DataXXX, size_t Count) \
79 { \
80    __asm__ __volatile__ ("rep stos{%z[DataXXX]|" z "}" \
81       : "+D" (Dest), "+c" (Count) \
82       : [DataXXX] "a" (DataXXX) \
83       : "memory"); \
84 }
```

Lines 159-163: Change `Data` to `DataXXX`

```
159 #define __buildwriteseg(x, y, z) void x(unsigned __LONG32 Offset, y DataXXX     ) { \
160     __asm__ ("mov{%z[offset] %[DataXXX], %%" z ":%[offset] | %%" z ":%[offs     et], %[DataXXX]}" \
161         : [offset] "=m" ((*(y *) (size_t) Offset)) \
162         : [DataXXX] "ri" (DataXXX)); \
163 }
```

