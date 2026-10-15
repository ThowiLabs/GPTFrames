#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
for dep in third_party/_sources/litehtml/CMakeLists.txt third_party/_sources/minih264/minih264e.h third_party/_sources/minimp4/minimp4.h third_party/_sources/vo-aacenc/common/include/voAAC.h; do
 if [ ! -f "$dep" ]; then
   echo "Falta $dep: use scripts/fetch-html-deps.sh y scripts/fetch-codecs.sh para obtener las fuentes" >&2
   exit 1
 fi
done
if [ ! -f font_embedded.hpp ]; then
 sh scripts/embed-fonts.sh
fi
cmake -S . -B build/remix-linux -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON -DHFC_BROWSER_CAPTURE=ON
cmake --build build/remix-linux -j3
cmake -S . -B build/remix-windows -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON -DHFC_BROWSER_CAPTURE=ON
cmake --build build/remix-windows -j3
echo "Linux y Windows listos: build/remix-linux/hyperframes-cpp-native y build/remix-windows/hyperframes-cpp-native.exe"
