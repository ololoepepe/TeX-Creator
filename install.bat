@echo off
echo Installing...
mkdir "%programfiles%\TeX Creator\autotext"
mkdir "%programfiles%\TeX Creator\layout-maps"
mkdir "%programfiles%\TeX Creator\plugins"
mkdir "%programfiles%\TeX Creator\translations"
mkdir "%programfiles%\TeX Creator\doc"
del "%programfiles%\TeX Creator\tex-creator.exe"
del "%programfiles%\TeX Creator\beqt*"
copy ".\build\release\tex-creator.exe" "%programfiles%\TeX Creator"
copy ".\install\layout-maps\*.klm" "%programfiles%\TeX Creator\layout-maps"
copy ".\install\translations\*.qm" "%programfiles%\TeX Creator\translations"
copy ".\install\doc\*" "%programfiles%\TeX Creator\doc"
copy ".\translations\*.qm" "%programfiles%\TeX Creator\translations"
copy "%programfiles%\BeQt\lib\beqtcore*" "%programfiles%\TeX Creator"
copy "%programfiles%\BeQt\lib\beqtgui*" "%programfiles%\TeX Creator"
copy "%programfiles%\BeQt\translations\*.qm" "%programfiles%\TeX Creator\translations"
echo Installation finished.
