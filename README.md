**Spearmint Quake 3** and **Team Arena** game code.

To use this you'll need the [Spearmint engine](https://github.com/zturtleman/spearmint).

  * On Windows, install [Cygwin and mingw-w64](https://github.com/zturtleman/spearmint/wiki/Compiling#windows).
  * Get the source for Spearmint and build it using `make`.
  * Get the source for this repo and build it using `make`.
  * Copy the pak?.pk3 files for baseq3 and missionpack into the directories in `mint-arena/build/release-mingw32-x86/`.
  * Copy the [spearmint-patch-data](https://github.com/zturtleman/spearmint-patch-data) for baseq3 and missionpack there too.

If you put both projects in the same directory you can launch the game using;

    spearmint/build/release-mingw32-x86/spearmint_x86.exe +set fs_basepath "mint-arena/build/release-mingw32-x86/" +set fs_game "baseq3"

On Linux and OS X you'll need to put `./` before the command and substitute the correct platform and architecture (look in the build directory).

## License

mint-arena is licensed under a [modified version of the GNU GPLv3](COPYING.txt#L625) (or at your option, any later version). This is due to including code from Return to Castle Wolfenstein and Wolfenstein: Enemy Territory.

Submitted contributions must be given with permission to use as GPLv**2** (two) and any later version; unless the file is under a license besides the GPL, in which case that license applies. This allows me to potentially change the license to GPLv2 or later in the future.
