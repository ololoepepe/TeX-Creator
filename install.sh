#!/bin/sh
echo "Installing..."
mkdir -p /usr/lib/tex-creator/qt4/plugins
mkdir -p /usr/lib/tex-creator/other
mkdir -p /usr/lib/tex-creator/beqt
mkdir -p /usr/lib/tex-creator/plugins
mkdir -p /usr/share/tex-creator/autotext
mkdir -p /usr/share/tex-creator/layout-maps
mkdir -p /usr/share/tex-creator/translations
rm /usr/lib/tex-creator/tex-creator
rm /usr/lib/tex-creator/beqt/libbeqtcore.*
rm /usr/lib/tex-creator/beqt/libbeqtgui.*
rm /usr/bin/tex-creator.sh
rm /usr/share/applications/tex-creator.desktop
rm /usr/share/pixmaps/tex-creator.png
cp ./build/tex-creator /usr/lib/tex-creator
cp -P /usr/lib/libbeqtcore.* /usr/lib/tex-creator/beqt
cp -P /usr/lib/libbeqtgui.* /usr/lib/tex-creator/beqt
cp /usr/share/beqt/translations/*.qm /usr/share/tex-creator/translations
cp ./install/tex-creator.sh /usr/bin
cp ./install/tex-creator.desktop /usr/share/applications
cp ./install/layout-maps/*.klm /usr/share/tex-creator/layout-maps
cp ./install/translations/*.qm /usr/share/tex-creator/translations
cp ./translations/*.qm /usr/share/tex-creator/translations
cp ./logo.png /usr/share/pixmaps/tex-creator.png
echo "Installation finished."
exit 0
