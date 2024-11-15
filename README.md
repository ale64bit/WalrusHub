# WalrusHub

A playground for weiqi tools and experimentation.

## Build instructions

The hub is written in C++ and uses [meson](https://mesonbuild.com/) as build system.
It depends on GTK4. See [here](https://www.gtk.org/docs/installations/#installations) for platform-specific installation instructions. On Linux and MacOS, it's usually available via your package manager.

Steps:

1. Fetch dependencies:
```
$ mkdir subprojects
$ meson wrap install curl
$ meson wrap install nlohmann_json
$ meson wrap install sqlite3
```

2. Create and setup build directory:
```
$ meson setup builddir
```

3. Compile and run:
```
$ meson compile -C builddir
$ ./builddir/WalrusHub
```
