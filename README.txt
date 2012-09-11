===============================================================================
 TeX Creator
===============================================================================

Homepage: https://github.com/the-dark-angel/TeX-Creator

Authors:

 * Yuri Melnikov
 * Andrey Bogdanov

License: See COPYING.txt

===============================================================================
 Requirements
===============================================================================

To build and use TeX Creator you will need the same libraries and tools as for
building any other Qt-based project.
See: http://qt-project.org/resources/getting_started for details.
You will also need the BeQt libraries (extension of Qt) version 0.1.x.
See: https://github.com/the-dark-angel/BeQt for details.

It is strongly recommended that you use Qt libraries 4.8.0 or higher. Further
releases of TeX Creator may use Qt libraries 5.0.
The easiest way to get Qt is to download the SDK.

===============================================================================
 Building and installing
===============================================================================

The first possibility is to use Qt Creator with the appropriate libraries, and
then use the install script only. In this case, you should specify the "build"
subdirectory as the target directory. After building the project, simply run
"./install.sh" (on UNIX-like systems) or "install.bat" (on Windows).

The other possibility is to use the terminal. You have to configure your "PATH"
variable correctly, or configure the terminal session to use the building
script. On Windows, for example, you may use the special configuration batch
files from the Qt SDK. After configuring the system environment (or terminal
session) do the following:

  1. On UNIX-like systems:

    In terminal, cd to the TeX Creator source files directory and run:
    "./build-install.sh"

  2. On Windows:

    In terminal, cd to the TeX Creator source files directory and run:
    "./build-install.sh [your_make_command] [make_parameters]", where
    your_make_command is the appropriate make command, or mingw32-make if no
    command is specified, and make_parameters is the list of make parameters
    (up to 4).

Note, that you will have to place Qt libraries (QtCore, QtGui, QtNetwork and
QtXml) manually to the following directories:

  1. On UNIX-like systems:

    "/usr/lib/tex-creator/qt4"

  2. On Windows:

    "%programfiles%\TeX Creator" (for example, "C:\Program files\TeX Creator")
