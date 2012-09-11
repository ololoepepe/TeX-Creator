#!/bin/sh
LD_LIBRARY_PATH="/usr/lib/tex-creator/qt4:/usr/lib/tex-creator/beqt:/usr/lib/tex-creator/other"
export LD_LIBRARY_PATH
/usr/lib/tex-creator/tex-creator "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"
