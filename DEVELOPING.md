
## Crosscompile for Windows (only tested for Ubuntu 18.04)

```
# build dependencies
sudo apt install -y clang g++ build-essential make mingw-w64 git pkg-config libc6-dev m4 g++-multilib autoconf libtool-bin ncurses-dev unzip python python-zmq zlib1g-dev wget curl bsdmainutils automake libgl1-mesa-dev libglu1-mesa-dev libfontconfig1-dev autopoint libssl-dev

# MXE dependencies
sudo apt install -y bash bison bzip2 flex gettext git gperf intltool libc6-dev-i386 libgdk-pixbuf2.0-dev libltdl-dev libtool-bin libxml-parser-perl make openssl p7zip-full patch perl pkg-config python ruby sed unzip wget xz-utils

```
## more dependencies 
```

apt-get -y update && apt-get install -y \
    curl libdbus-1-3 libexpat1 \
    libgl1-mesa-dev libglu1-mesa-dev libfontconfig1-dev libssl-dev  \
    libfreetype6 libgl1-mesa-glx libglib2.0-0 \
    libx11-6 libx11-xcb1 \
    g++ build-essential cmake wget git clang++-6.0 \
    software-properties-common  \
    autoconf automake autopoint bison flex gperf libtool libtool-bin intltool lzip python ruby unzip p7zip-full libgdk-pixbuf2.0-dev libltdl-dev
    
  sudo apt-get build-dep qt5-default
sudo apt-get install libxcb-xinerama0-dev 
```
# Compile OpenSSL

```
# Download openssl 1.1.1g from https://openssl.org/source/

wget https://www.openssl.org/source/openssl-1.0.2t.tar.gz
tar zxvpf openssl-1.1.1g.tar.gz
cd openssl-1.0.2t
./config # linux-x86_64
make -j$(nproc)
```
## Static build of Qt5

# Download Qt5 sources
```
mkdir -p ~/Qt/5.15.0 && cd ~/Qt/5.15.0

wget https://download.qt.io/archive/qt/5.15/5.15.0/single/qt-everywhere-src-5.15.0.tar.xz
tar xvpf qt-everywhere-src-5.15.0.tar.xz && cd qt-everywhere-src-5.15.0

```
# Configure and build Qt5 statically.
```
OPENSSL_LIBS='-L/PATH/TO/openssl-1.0.2q -lssl -lcrypto' ./configure -static -prefix ~/Qt/5.11.2/static  -skip qtlocation -skip qtmacextras -skip qtpurchasing -skip qtscript -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qtdatavis3d -skip qtdoc -skip qtcharts -skip qtdeclarative -skip qt3d -skip qtwebengine -skip qtandroidextras -skip qtwebview -skip qtgamepad -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects -skip qtwebview -skip qtwebchannel -skip qtwebglplugin  -nomake examples -nomake tests -qt-zlib -qt-libpng -xcb -xkbcommon -feature-fontconfig -no-feature-getentropy  -release -openssl-linked -opensource

make -j$(nproc)
make -j$(nproc) install

```
# Build MXE (Cross-compiled Qt5 for Windows in Linux)
```
mkdir ~/github && cd ~/github
git clone https://github.com/mxe/mxe.git
cd mxe

make -j$(nproc) MXE_TARGETS=x86_64-w64-mingw32.static qtbase qtwebsockets

```
# Build SilentDragonLite .exe
```
cd SilentDragonLite
curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain 1.38.0  -y 
echo 'source $HOME/.cargo/env' >> $HOME/.bashrc
 ~/.cargo/bin/rustup target add x86_64-pc-windows-gnu
 echo "[target.x86_64-pc-windows-gnu]" >> ~/.cargo/config
 echo "linker = 'x86_64-w64-mingw32.static-gcc'" >> ~/.cargo/config

./win-static-build.sh

```
