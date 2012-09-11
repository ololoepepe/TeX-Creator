@echo off
echo Installing...
mkdir "%programfiles%\TeX Creator\plugins"
mkdir "%programfiles%\TeX Creator\macros"
copy .\build\release\tex-creator.exe "%programfiles%\TeX Creator"
copy "%programfiles%\BeQt\lib\beqtcore0.dll" "%programfiles%\TeX Creator"
copy "%programfiles%\BeQt\lib\beqtgui0.dll" "%programfiles%\TeX Creator"
copy "%programfiles%\BeQt\lib\beqtnetwork0.dll" "%programfiles%\TeX Creator"
echo Installation finished.
