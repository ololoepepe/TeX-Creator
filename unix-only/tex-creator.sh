#!/bin/sh
export LD_LIBRARY_PATH="/usr/lib/tex-creator"
exec /usr/lib/tex-creator/tex-creator "$@"
