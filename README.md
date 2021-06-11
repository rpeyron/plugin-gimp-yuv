# plugin-gimp-yuv
GIMP plugin to convert RGB to YUV

Plugin GIMP : YUV Transforms

## What it does

It allows you to transform your RBG image in YUV, or your YUV image in RGB.  The Y,U,V components will be stored in the R,G,B layers in GIMP, so it will be displayed oddly, but wll allow you to work on each layer, and then revert to RGB.

![image](https://user-images.githubusercontent.com/3126751/121743407-9a5b1b80-cb01-11eb-8f3e-74f4a6ea948c.png)

## Use 

It adds menu items : 
*  Image/Colors/RGB->YUV
*  Image/Colors/YUV->RGB

## Installation

### Linux

You will need the development packages of gimp and glib  
For instance, on debian/ubuntu : `sudo apt-get install libgimp2.0-dev`

Then:
```
make
make install
```

### Windows
 
Binaries for windows are provided as separate packages. Please download the 32bits or 64bits according to you GIMP version (this is not related to Windows version). Altough the GIMP API is quite stable, the binaries are not, and the plugin binaries must be updated to new GIMP versions (some will work, some won't). The GIMP version is indicated in the package filename. Download the binaries that fits the best to your GIMP version. Just copy the files (fourier.exe and libfftw3-3.dll) in the plugins directory of eiher:
- your personal gimp directory (ex: .gimp-2.2\plug-ins), 
- or in the global directory (C:\Program Files\GIMP-2.2\lib\gimp\2.0\plug-ins)

You can also build with msys2 environment:
```
msys2 -c "pacman -S --noconfirm mingw-w64-i686-toolchain"
msys2 -c "pacman -S --noconfirm mingw-w64-i686-gimp=2.10.24"
msys2 -mingw32 -c 'echo $(gimptool-2.0 -n --build yuv.c) -O3 | sh'
```
This is for 32bits version ; replace i686 by x84_64 and -mingw32 by -mingw64 if you want 64bits. Replace also 2.10.24 by your GIMP version (or leave empty for latest version)

Also, the windows binaries are build through GitHub Actions. So you may also fork this repository and build the plugin on your own.
