Build installable packages with the root CMake project:

    cmake --build build/mingw-release --target package_dcext

CMake configures info.xml.in for the selected architecture, then packages
info.xml, MediaPlayer-<arch>.dll and MediaPlayer.ico into a ZIP-format .dcext.
See ../README for build, test and installation instructions.

info.xml and info_generated.xml are historical manual-packaging examples;
they are not used by CMake. The packager/ helper belongs to the old projects/.
