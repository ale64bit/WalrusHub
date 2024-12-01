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

3. Unzip the task database. For example, on *nix systems:
```
$ unzip assets/tasks.zip -d assets/
```

4. Compile and run:
```
$ meson compile -C builddir
$ ./builddir/WalrusHub
```

## Screenshots

### Editor
![Alt text](doc/editor_screenshot.png?raw=true "Editor")
### Play against KataGo human-like AI
![Alt text](doc/play_ai_screenshot.png?raw=true "Play against KataGo")
### Solve 101weiqi tasks in time-challenge (a.k.a. guan) mode
![Alt text](doc/time_challenge_screenshot.png?raw=true "Time challenge solving")
### Solve 101weiqi tasks in training mode
![Alt text](doc/training_screenshot.png?raw=true "Training")
### Keep track of your solve stats
![Alt text](doc/stats_screenshot.png?raw=true "Statistics")
