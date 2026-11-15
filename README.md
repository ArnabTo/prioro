# Prioro

C++17 terminal task manager. Spec: `prioro-context.md`. Docs: [`docs/PROJECT.md`](docs/PROJECT.md) (overview), [`docs/CODE.md`](docs/CODE.md) (files + execution flow).

## Build

```bat
set PATH=C:\msys64\mingw64\bin;%PATH%
build.bat
build\prioro.exe
```

Linux / WSL: `./build.sh` then `./build/prioro`

## Code layout (simple)

```
src/prioro.h    Task struct + function declarations
src/prioro.cpp  tasks, file save/load, graph, recommend, stats
src/main.cpp    menu only
data/           tasks.txt, deps.txt
```

No classes except `Task`. Fixed arrays (`MAX_TASKS` 100). Plain functions.
