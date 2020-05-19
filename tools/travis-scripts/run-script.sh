#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
COCOS2DX_ROOT="$DIR"/../..

if [ -z "$NDK_ROOT" ]; then
    export NDK_ROOT=$HOME/bin/android-ndk
fi

# to fix git error: shallow update not allowed
# https://stackoverflow.com/questions/28983842/remote-rejected-shallow-update-not-allowed-after-changing-git-remote-url
#git remote add old https://github.com/cocos-creator/cocos2d-x-lite
#git fetch --unshallow old

set -x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-bindings.sh $TRAVIS_BRANCH


mkdir -p $HOME/android

ANDROID_SDK=$HOME/android/android_sdk
export ANDROID_HOME=$ANDROID_SDK
export ANDROID_NDK=$NDK_ROOT       #installed in generate-bindings.sh
export ANDROID_NDK_HOME=$NDK_ROOT


function setup_linux_andorid_sdk()
{
    echo "Download Android SDK... "
    cmake_dir=$ANDROID_SDK/cmake/3.10.2.4988404/bin
    cd $HOME/android
    if ! [ -d "$cmake_bin" ]; then
        wget -t 5 -q https://dl.google.com/android/repository/commandlinetools-linux-6200805_latest.zip
        unzip *.zip
        yes | ./tools/bin/sdkmanager  --sdk_root="$ANDROID_SDK" \
                "platforms;android-27" \
                "build-tools;28.0.3" \
                "platform-tools" \
                "tools"  \
                "cmake;3.10.2.4988404"
    else
        echo " Android SDK found! Skip downloading!"
    fi
    export PATH=$cmake_dir:$PATH
}

function build_android()
{
    echo "Compiling Android ... "
    cd $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src/proj.android-studio
    sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" app/build.gradle
    sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" settings.gradle
    sed -i "s/^RELEASE_/#RELEASE_/g" gradle.properties

    #echo "Compile Android - ndk-build ..."
    #./gradlew assembleDebug --quiet
    
    echo "Compile Android - cmake ..."
    echo "ANDORID_NDK ${ANDROID_NDK} or ${ANDROID_NDK_HOME}" 
    ./gradlew assembleDebug --quiet
    echo "Compile Android Done!"
}

function mac_install_cmake()
{   
    echo "Compiling CMake ... "
    if ! [ -d $HOME/bin/cmake/bin ]; then
        cd $HOME/bin
        NUM_OF_CORES=`getconf _NPROCESSORS_ONLN`
        cmake_source=https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0.tar.gz
        wget -t 5 --no-check-certificate $cmake_source -O cmake-mac.tar.gz -q
        tar xf cmake-mac.tar.gz 2>/dev/null
        cd cmake-3.17.0
        ./configure --prefix=$HOME/bin/cmake > /dev/null
        make -j $NUM_OF_CORES >/dev/null
        make install >/dev/null
        ls $HOME/bin/cmake
    else
        echo " CMake found! Skip Compiling!"
    fi

    export PATH=$HOME/bin/cmake/bin:$PATH
}

function mac_download_cmake()
{
    echo "Download CMake ..."
    if ! [ -f cmake_bin.tar.gz ]; then
        cmake_binary=https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0-Darwin-x86_64.tar.gz
        wget -t 3 --no-check-certificate $cmake_binary -O cmake_bin.tar.gz -q
        tar xf cmake_bin.tar.gz 2>/dev/null
    else
        echo " CMake found! Skip downloading!"
    fi
    cmake_bin_dir=`dirname $(find . -name cmake-gui)`
    cmake_bin_dir="$PWD/$cmake_bin_dir"
    export PATH=$cmake_bin_dir:$PATH
}

function build_macosx()
{
    NUM_OF_CORES=`getconf _NPROCESSORS_ONLN`

    echo "Compile build/cocos2d_libs.xcodeproj ..."
    cd $COCOS2DX_ROOT/build
    xcodebuild -project cocos2d_libs.xcodeproj -target "libcocos2d Mac" -jobs $NUM_OF_CORES -quiet build 

    echo "Compiling MacOSX ... "
    cd  $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src
    mkdir build-mac 
    cd build-mac
    cmake .. -GXcode -DCOCOS_X_ROOT=$COCOS2DX_ROOT
    cmake --build . --config Release -- -quiet -jobs $NUM_OF_CORES
    echo "Compile MacOSX Done!"
}

function build_ios()
{
    NUM_OF_CORES=`getconf _NPROCESSORS_ONLN`

    echo "Compile build/cocos2d_libs.xcodeproj ..."
    cd $COCOS2DX_ROOT/build
    xcodebuild -project cocos2d_libs.xcodeproj -target "libcocos2d iOS" -jobs $NUM_OF_CORES -quiet build 

    echo "Compiling iOS ... "
    cd  $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src
    mkdir build-ios 
    cd build-ios
    cmake .. -GXcode -DCOCOS_X_ROOT=$COCOS2DX_ROOT -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphonesimulator \
        -DUSE_SE_JSC=ON
    cmake --build . --config Debug -- -quiet -jobs $NUM_OF_CORES
    echo "Compile iOS Done!"
}

function build_windows()
{
    echo "Compile build/libcocos2d.vcxproj ..."

    if ! [ -x "$(command -v MSBuild)" ]; then
        echo 'Error: MSBuild is not located!' 
    else
        cd $COCOS2DX_ROOT/build
        MSBuild /nologo /t:Build libcocos2d.vcxproj
    fi

    echo "Compiling Win32 ... "
    cd  $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src
    mkdir build-win32 
    cd build-win32
    cmake .. -G"Visual Studio 15 2017" -DCOCOS_X_ROOT=$COCOS2DX_ROOT 
    cmake --build . --config Debug 
    echo "Compile Win32 Done!"
}


function run_compile()
{
    if [ "$BUILD_TARGET" == "android_cmake" ]; then
        setup_linux_andorid_sdk
        build_android
    fi

    if [ "$BUILD_TARGET" == "macosx_cmake" ]; then
        mac_download_cmake
        build_macosx
    fi

    if [ "$BUILD_TARGET" == "ios_cmake" ]; then
        mac_download_cmake
        build_ios
    fi

    if [ "$BUILD_TARGET" == "windows_cmake" ]; then
        cmake --version
        build_windows
    fi
}

run_compile

cd $COCOS2DX_ROOT
## revert change
git clean -fdx templates
git checkout HEAD templates
set +x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-cocosfiles.sh $TRAVIS_BRANCH
exit 0
