#!/bin/sh
echo "Building..."
cd build
qmake CONFIG+="release" ../tex-creator.pro
make "$@"
echo "Building finished."
cd ..
./install.sh
exit 0
