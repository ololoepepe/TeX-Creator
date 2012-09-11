#!/bin/sh
echo "Installing..."
mkdir -p /usr/lib/tex-creator/qt4/plugins
mkdir -p /usr/lib/tex-creator/other
mkdir -p /usr/lib/tex-creator/beqt
mkdir -p /usr/lib/tex-creator/plugins
mkdir -p /usr/share/tex-creator/macros
cp ./build/tex-creator /usr/lib/tex-creator
cp ./install/tex-creator.sh /usr/bin
cp ./install/tex-creator.desktop /usr/share/applications
cp ./logo.png /usr/share/pixmaps/tex-creator.png
cp /usr/lib/libbeqtcore.so.0 /usr/lib/tex-creator/beqt
cp /usr/lib/libbeqtgui.so.0 /usr/lib/tex-creator/beqt
cp /usr/lib/libbeqtnetwork.so.0 /usr/lib/tex-creator/beqt
echo "Installation finished."
exit 0
