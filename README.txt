===============================================================================
 TeX Creator
===============================================================================

Homepage: https://github.com/TeXSample-Team/TeX-Creator

Author: TeXSample Team

License: GNU GPLv3 (see COPYING.txt)

TeX Creator is a LaTeX editor with a built-in TeXSample service client.

Binary packages are available at: http://sourceforge.net/projects/tex-creator

==============================================================================
 Overview
==============================================================================

TeX Creator is a simple LaTeX editor, which has all the necessary features
of the text editor. It is also a client of the TeXSample service.
TeX Creator is designed both for beginners and for professionals.

TeXSample is an online service, which is for use in the environment of schools
and institutions of higher education. With it, users can share the logically
complete fragments of textbooks, such as theorems, examples, definitions, etc.

In addition, TeXSample gives users a full-featured on-line compiler,
which works just the same as a usual local compiler.

==============================================================================
 Dependencies
==============================================================================

In order to build and use TeX Creator you will need the same libraries and
tools as for every other project using Qt.

See: http://qt-project.org/resources/getting_started for details.

TeX Creator is intended for use with Qt 5, or with Qt 4.8.
In case of using Qt 4.8, some features may be missing.
Support of Qt libraries' versions lower than 4.8.0 is not guaranteed.

In order to use builtin .svgz icons you will also need
the corresponding plugin.

See: http://qt-project.org/doc/qt-5.0/plugins-howto.html for details.

You will also need BeQt libraries version 2.1.0 or higher
and TeXSample library version 0.2.0 or higher.

See: https://github.com/the-dark-angel/BeQt
and https://github.com/TeXSample-Team/TeXSample for details.

==============================================================================
 Building and installing (briefly)
==============================================================================

In order to build TeX Creator, just cd into the sources directory and execute
the following commands:

 * "qmake [BEQT_PREFIX=<path>] [TSMP_PREFIX=<path>]"
   Here, the path to BeQt and TeXSample may be specified

 * "make"
   or other similar command ("nmake", "mingw32-make", etc.)

 * "make install"
   You may need the superuser privileges in UNIX-like systems 

Note: When building from the command line, you have to configure
the environment (path to Qt libraries, etc.).

You may also use Qt Creator. After building the project, cd to
the output directory and execute the "make install" command, or
configure automatic execution of that command in Qt Creator.

See: http://qt-project.org/doc/qtcreator-2.6 for details.

==============================================================================
 Building and installing (in detail)
==============================================================================

When building TeX Creator, you may pass some parameters to qmake:

 * "TCRT_CONFIG+=builtin_resources"
   Embed resources (including translations) into executable file
   See: http://qt-project.org/doc/qt-5.0/resources.html for details

 * "TCRT_CONFIG+=no_install"
   Don't install any files (building only)

 * "BEQT_PREFIX=<path>"
   Set path to BeQt libraries. Must be used if BeQt libraries were
   installed to a custom location

 * "TSMP_PREFIX=<path>"
   Set path to TeXSample libraries. Must be used if TeXSample libraries were
   installed to a custom location

 * "PREFIX=<path>"
   Set install path. See the note below

 * "BINARY_INSTALLS_PATH=<path>"
   Set install path for executable file. See the note below

 * "RESOURCES_INSTALLS_PATH=<path>"
   Set install path for resources. See the note below

Note: In Windows systems TeX Creator is installed to the
"C:\Program files\TeX-Creator" directory (or other similar directory,
depending on your system) by default. Executable file and resources are
installed to the corresponding subdirectories.

In UNIX-like systems the directory structure during installation
looks like this:

 * "/usr/bin/tex-creator"
   Path to executable file

 * "/usr/share/tex-creator"
   Path to resources

You may set another installation path. To do so,
pass the corresponding parameter to qmake (see above).

Warning: Don't use paths containing spaces. In Windows systems you may replace
directory names, containing spaces, with the corresponding short names:

See: http://support.microsoft.com/kb/142982 for details.

==============================================================================
 FAQ
==============================================================================

Q: What are TeX Creator license restrictions?
A: There are almost no restrictions. You may use TeX Creator as you wish,
but don't forget that this statement doesn't apply to the Qt libraries.
See: COPYING.txt, http://qt-project.org/products/licensing for details.

Q: I'm having troubles using TeX Creator, where can I get help?
A: E-mail/Jabber: ololoepepe@gmail.com

Q: I've detected a bug/I have an idea, where can I report/post it?
A: See the answer above.

==============================================================================
 Deploying
==============================================================================

When deploying TeX Creator, don't forget to include the resource files
(images, translations, etc. - see above).

It's recommended to build applications statically, because that helps avoid
dependency issues. You may use the "builtin_resources" parameter
in case of building your project statically.

For details, see:
http://qt-project.org/doc/qt-5.0/deployment.html#static-vs-shared-libraries
