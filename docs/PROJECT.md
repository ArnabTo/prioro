# Prioro — Project Documentation

Guide for understanding the code and explaining it to your teacher.

---

## What Prioro is

Prioro is a **terminal task manager**. It is not sorted by deadline only.

It suggests what to do **now** using:

- **Energy** (low / medium / high) and **place** (quiet / noisy / internet) from a short check-in
- **Dependencies** (task B waits until task A is finished)
- **Postpone count** (`rescheduleCount`) as time-debt — higher score = more urgent

Tasks save locally in `data/tasks.txt` and `data/deps.txt`. No login, no cloud for the core app.

**Menu option 6 (Chat)** is a placeholder until an online AI API is added later.

---

## Folder structure (current project)

```
prioro-group-proj/
  prioro-context.md    full product spec (team reference)
  README.md            build commands
  build.bat            Windows compile script
  build.sh             Linux / WSL compile script
  .gitignore           ignores build/ output
  src/
    prioro.h           Task struct + public function declarations
    prioro.cpp         all business logic
    main.cpp           menu and user input only
  data/
    tasks.txt          saved tasks (created/updated by app)
    deps.txt           dependency edges
  docs/
    PROJECT.md         this file
  build/
    prioro.exe         compiled program (run build.bat; not stored in git)
```

There are **no** separate `include/` or `core/` folders — everything is three files under `src/`.

---

## How the code is split

| File | Role |
|------|------|
| **`main.cpp`** | Prints menu, reads choices, calls functions from `prioro.h`. No task rules here. |
| **`prioro.h`** | `Task` struct, limits (`MAX_TASKS` 100, `MAX_DEPS` 200), function prototypes. |
| **`prioro.cpp`** | Arrays in memory, load/save files, dependencies, recommendations, stats. |

**Design rule:** Teacher can read `main.cpp` for UI and `prioro.cpp` for logic.

---

## `Task` and state values

```cpp
struct Task {
  int id;
  std::string title, category, deadline;
  int energy;   // 0=low, 1=medium, 2=high
  int env;      // 0=quiet, 1=noisy, 2=internet
  bool completed;
  int rescheduleCount;
};
```

`UserState` holds the user’s current `energy` and `env` (same numbering) after check-in on menu option 2.

Helper functions in `prioro.h`: `prioroParseEnergy`, `prioroParseEnv`, `prioroEnergyName`, `prioroEnvName`.

---

## Menu → code flow

| Menu | `main.cpp` | Calls in `prioro.cpp` |
|------|------------|-------------------------|
| 1 Add task | `handleAdd()` | `prioroAddTask` |
| 2 What now? | `handleNow()` | `prioroRecommend` |
| 3 View all | `handleView()` | `prioroTaskCount`, `prioroGetTaskByIndex` |
| 4 Complete | `handleComplete()` | `prioroCompleteTask` |
| 5 Stats | `handleStats()` | `prioroStats`, `prioroPrintCategoryStats` |
| 6 Chat | `handleChat()` | stub message only |
| 7 Exit | `main` loop ends | — |

On startup, `main` calls **`prioroLoad()`** once. After add/complete, **`prioroSave()`** runs inside `prioro.cpp`.

---

## Public functions (`prioro.h`)

| Function | Purpose |
|----------|---------|
| `prioroLoad()` | Read `data/tasks.txt` and `data/deps.txt` into memory |
| `prioroSave()` | Write both files (creates `data/` if needed) |
| `prioroFindTask(id)` | Return pointer to task or `NULL` |
| `prioroTaskCount()` | Number of tasks in array |
| `prioroGetTaskByIndex(i)` | Task at index `0 .. count-1` |
| `prioroAddTask(task, dependsOnId, error)` | New id, optional dependency, save |
| `prioroCompleteTask(id, error)` | Mark completed, save |
| `prioroRecommend(state, ids[], maxIds)` | Fill `ids` with best task ids; return count |
| `prioroStats(...)` | Totals: all, done, pending, high postpone (≥3) |
| `prioroPrintCategoryStats()` | Print completed count per category |

---

## Internal logic (`prioro.cpp` only)

These are **not** in the header; they implement the graph and ranking:

| Name | Purpose |
|------|---------|
| `g_tasks[]`, `g_deps[]` | Fixed-size arrays in memory |
| `addDepEdge` | Add dependency; check duplicates |
| `wouldCycle` / `reachesImpl` | **DFS** — block if prerequisite chain would loop |
| `isBlocked` | Any prerequisite not completed? |
| `scoreTask` | Higher postpone + deadline text + high energy → higher score |
| `sortScoresDesc` | **Bubble sort** ids by score (simple to explain in viva) |
| `energyFits` / `envFits` | Can user do this task in current state? |

---

## Data structures (DSA — how we explain them)

| Topic | Implementation |
|-------|----------------|
| List of tasks | Array `g_tasks[MAX_TASKS]` + `g_taskCount` |
| Lookup by id | Linear scan in `prioroFindTask` |
| Graph (dependencies) | List of edges `(dependent, prerequisite)` in `g_deps[]` |
| Cycle detection | DFS from prerequisite toward dependent |
| Priority / ranking | Numeric score + bubble sort (not `priority_queue`) |
| Persistence | Text files, pipe `\|` separated fields |

This matches course topics (arrays, graph, DFS, sorting, files) without advanced C++ libraries.

---

## File formats

**`data/tasks.txt`** — one task per line (lines starting with `#` ignored):

```
id|title|category|deadline|energy|env|completed|reschedule
```

- `energy` / `env`: integers 0–2 as above  
- `completed`: `0` or `1`  
- Avoid `\|` in titles (simple format)

**`data/deps.txt`** — one dependency per line:

```
dependentId|prerequisiteId
```

Meaning: **dependent** cannot start until **prerequisite** is completed.

---

## Technologies

| Item | Use |
|------|-----|
| C++17 | Language |
| `iostream`, `fstream`, `sstream` | Menu and files |
| `string` | Task text fields |
| g++ via `build.bat` / `build.sh` | Compile two `.cpp` files + `-static` on Windows |
| No CMake, no classes (except structs), no STL maps/heaps |

---

## Build and run

From **project root** (so `data/` paths work):

**Windows:**

```bat
set PATH=C:\msys64\mingw64\bin;%PATH%
build.bat
build\prioro.exe
```

**Linux / WSL / macOS:**

```bash
chmod +x build.sh
./build.sh
./build/prioro
```

`build/` can be deleted anytime; rebuild with `build.bat` or `build.sh`.

---

## What is done vs planned

**Done now**

- Menu UI (7 options)
- Add / view / complete tasks
- Dependencies with cycle check
- Check-in + ranked suggestions
- Basic stats + per-category completed counts
- Local file storage

**Planned later** (see `prioro-context.md`)

- AI chat and natural-language add (libcurl + API)
- CLI flags (`prioro add ...`)
- `prioro plan --time 90` (knapsack)
- Smarter deadlines and pattern learning

---

## Demo script for teacher (~5 minutes)

1. **Add** task 1: “Read chapter” — low energy, quiet.  
2. **Add** task 2: “Write report” — high energy, quiet — **depends on id 1**.  
3. **What now?** — low energy → only task 1.  
4. **Complete** task 1 → **What now?** — high energy → task 2.  
5. Show **Stats**.  
6. Explain: data in `data/`, logic in `prioro.cpp`, menu in `main.cpp`.

---

## Team topics (optional mapping)

| Area | Where in code |
|------|----------------|
| Data / files | `prioroLoad`, `prioroSave` |
| Graph / deps | `g_deps`, `wouldCycle`, `isBlocked` |
| Recommendations | `prioroRecommend`, `scoreTask` |
| CLI / menu | `main.cpp` |
| AI | menu 6 stub |

For full vision and future features, read **`prioro-context.md`**.

**Code walkthrough (files, connections, step-by-step menu flow):** [`CODE.md`](CODE.md)
