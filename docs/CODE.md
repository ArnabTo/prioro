# Prioro — Code Documentation

How the source files and `data/` folder work together, and what happens from program start to each menu action.

---

## 1. Big picture

```
  ┌─────────────┐         #include          ┌─────────────┐
  │  main.cpp   │ ────────────────────────► │  prioro.h   │
  │  (menu UI)  │     calls functions       │  (contract)│
  └──────┬──────┘                           └──────▲──────┘
         │                                          │
         │                              implements │
         │                                          │
         └──────────────────────────────────────────┤
                                                    │
                                            ┌───────┴───────┐
                                            │  prioro.cpp   │
                                            │  (logic + RAM)│
                                            └───────┬───────┘
                                                    │
                              read / write          │
                                                    ▼
                                            ┌───────────────┐
                                            │  data/        │
                                            │  tasks.txt    │
                                            │  deps.txt     │
                                            └───────────────┘
```

- **`prioro.h`** — shared definitions: what a `Task` is, which functions exist.
- **`prioro.cpp`** — real work: arrays in memory, rules, reading/writing disk.
- **`main.cpp`** — only talks to the user (print menu, read numbers/text).
- **`data/`** — long-term storage; survives after you close the program.

Compile links **`main.cpp` + `prioro.cpp`** into one executable (`build/prioro.exe`).

---

## 2. What each part is for

### `prioro.h` — header (the “menu of functions”)

**Use:** So `main.cpp` knows the types and function names without seeing all the implementation.

**Contains:**

- Limits: `MAX_TASKS` (100), `MAX_DEPS` (200)
- `struct Task` — one activity (id, title, category, deadline, energy, env, completed, postpone count)
- `struct UserState` — user’s current energy and environment after check-in
- Declarations like `prioroLoad()`, `prioroAddTask(...)`, `prioroRecommend(...)`

**Does not contain:** loops over files, graph logic, or menu text.

---

### `prioro.cpp` — implementation (the “engine”)

**Use:** Holds all tasks and dependencies in RAM and enforces rules.

**In memory (private to this file):**

- `g_tasks[]` — array of tasks
- `g_taskCount` — how many tasks are used
- `g_nextId` — next id for a new task
- `g_deps[]` — list of “B depends on A” pairs
- `g_depCount` — how many dependency edges exist

**Talks to disk:**

- `prioroLoad()` — opens `data/tasks.txt` and `data/deps.txt`, fills arrays
- `prioroSave()` — writes arrays back to those files (creates `data/` folder if missing)

**Main jobs:**

| Job | Functions |
|-----|-----------|
| Parse words → numbers | `prioroParseEnergy`, `prioroParseEnv` |
| Find / list tasks | `prioroFindTask`, `prioroTaskCount`, `prioroGetTaskByIndex` |
| Add / complete | `prioroAddTask`, `prioroCompleteTask` |
| Dependencies + no cycles | `addDepEdge`, `wouldCycle`, `reachesImpl`, `isBlocked` |
| Suggest what to do | `prioroRecommend`, `scoreTask`, `sortScoresDesc` |
| Statistics | `prioroStats`, `prioroPrintCategoryStats` |

---

### `main.cpp` — user interface (the “face” of the app)

**Use:** Show Prioro banner and menu; read user choice; call the right functions from `prioro.h`.

**Includes only:** `#include "prioro.h"` plus iostream for `cin`/`cout`.

**Helper functions (local to main.cpp):**

- `readLine`, `readInt` — input
- `printHeader`, `printMenu`, `printTask` — output
- `handleAdd`, `handleNow`, `handleView`, `handleComplete`, `handleStats`, `handleChat` — one per menu option

**Does not:** read `tasks.txt` directly or implement graph algorithms.

---

### `data/` — persistent storage (the “notebook on disk”)

**Use:** Keep tasks and dependencies when the program is not running.

| File | Role |
|------|------|
| **`tasks.txt`** | One line per task; fields separated by `\|` |
| **`deps.txt`** | One line per dependency: `dependentId\|prerequisiteId` |

Example task line (from your project):

```
1|Complete the versity task|Importatn|tomorrow|1|0|0|0
```

Meaning: id=1, energy=1 (medium), env=0 (quiet), not completed, postpone count 0.

Lines starting with `#` are comments (headers).

**Important:** Run `prioro.exe` from the **project root** so paths `data/tasks.txt` and `data/deps.txt` are correct.

---

## 3. How the files connect at compile time

1. `g++` compiles `prioro.cpp` → object file (implements everything declared in `prioro.h`).
2. `g++` compiles `main.cpp` → object file (includes `prioro.h`, calls those functions).
3. Linker joins both into **`build/prioro.exe`**.

At run time there is **one program**. `main.cpp` and `prioro.cpp` are not separate apps; they are two modules linked together.

---

## 4. Execution from start to end

### Phase A — Program starts

```
main() starts
    │
    ▼
prioroLoad()                    [prioro.cpp]
    │
    ├─► Open data/tasks.txt (if missing, start with 0 tasks)
    │       Parse each line → push into g_tasks[], update g_nextId
    │
    └─► Open data/deps.txt (if missing, 0 dependencies)
            Parse each line → add to g_deps[]
    │
    ▼
Enter while(running) menu loop     [main.cpp]
```

Nothing is saved yet on startup — only **read**.

---

### Phase B — Menu loop (repeats until Exit)

Each lap:

1. `printHeader()` + `printMenu()`
2. User enters choice (1–7)
3. `switch(choice)` → one `handle...()` function
4. `readLine("Press Enter...")` unless user chose Exit

---

### Phase C — What each activity does (step by step)

#### Activity 1 — Add a task

```
handleAdd()                           [main.cpp]
    read title, category, deadline, energy, env, optional dep id
    build a Task struct
    │
    ▼
prioroAddTask(task, depId, error)     [prioro.cpp]
    assign new id from g_nextId++
    append to g_tasks[]
    if depId > 0:
        check prerequisite exists
        addDepEdge → wouldCycle (DFS) must pass
    │
    ▼
prioroSave()                          [prioro.cpp]
    write data/tasks.txt + data/deps.txt
    │
    ▼
main prints "Task added with id N"
```

---

#### Activity 2 — What should I do now?

```
handleNow()                           [main.cpp]
    promptCheckIn → UserState (energy, env)
    │
    ▼
prioroRecommend(state, ids[], 10)     [prioro.cpp]
    for each task in g_tasks:
        skip if completed
        skip if energy/env don't fit user state
        skip if isBlocked (prerequisite not done)
        else compute scoreTask → collect candidates
    bubble sort by score (highest first)
    copy top ids into ids[]
    return count
    │
    ▼
main loops ids[], prioroFindTask(id), printTask()
```

---

#### Activity 3 — View all tasks

```
handleView()                          [main.cpp]
    prioroTaskCount()
    for i = 0 .. count-1:
        prioroGetTaskByIndex(i) → printTask()
```

No file read — data already in RAM from last `prioroLoad` / last save.

---

#### Activity 4 — Mark complete

```
handleComplete()                      [main.cpp]
    read task id
    │
    ▼
prioroCompleteTask(id, error)         [prioro.cpp]
    prioroFindTask(id) → set completed = true
    prioroSave() → update data/tasks.txt
```

After this, **`isBlocked`** may unblock tasks that depended on this id.

---

#### Activity 5 — View stats

```
handleStats()                         [main.cpp]
    prioroStats(...) → totals
    prioroPrintCategoryStats()        [prioro.cpp prints to cout]
```

---

#### Activity 6 — Chat (stub)

```
handleChat()                          [main.cpp only]
    read message
    print offline stub (no prioro.cpp AI yet)
```

---

#### Activity 7 — Exit

```
running = false
loop ends
print "Goodbye."
return 0
```

**Note:** Data is already saved after each add/complete. Exit does not need an extra save.

---

## 5. Data flow summary

| When | RAM (`prioro.cpp`) | Disk (`data/`) |
|------|--------------------|----------------|
| App start | `prioroLoad` fills arrays | Read `tasks.txt`, `deps.txt` |
| Add / complete | Arrays updated | `prioroSave` writes both files |
| View / recommend / stats | Read arrays only | No access |

---

## 6. One sentence for each file (for viva)

- **`prioro.h`** — Defines `Task` and lists the functions `main` is allowed to call.
- **`prioro.cpp`** — Stores tasks in arrays, applies rules, loads and saves `data/`.
- **`main.cpp`** — Shows the menu and calls `prioro.h` functions based on user choice.
- **`data/`** — Text files that remember tasks and dependencies between runs.

---

## 7. Related docs

- Project overview and DSA: **`PROJECT.md`**
- Product vision: **`prioro-context.md`**
- Build commands: **`README.md`**
