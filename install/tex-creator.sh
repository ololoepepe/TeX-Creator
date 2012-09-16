#!/bin/sh
export LD_LIBRARY_PATH="/usr/lib/tex-creator/qt4:/usr/lib/tex-creator/beqt:/usr/lib/tex-creator/other"
exec /usr/lib/tex-creator/tex-creator "$@"
