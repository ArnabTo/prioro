# Prioro — Project Context

An AI-powered, state-aware personal activity management system, built first as a **C++ terminal application**. This file is the full project context — use it to generate the first terminal menu/UI screen.

## 1. What Prioro Is

Prioro is not a plain to-do list. It recommends tasks based on:
- The user's **current state** (energy level + environment), checked in manually
- **Task dependencies** (some tasks can't start until others are done)
- **Behavioral patterns** (tasks the user keeps rescheduling / avoiding)

It also has an **optional AI layer** (needs internet) that lets the user add tasks in plain English and chat with the app for suggestions — like a partner, not a checklist.

The core engine works 100% offline. A graphical UI may be added later on top of the same core logic — so core logic and terminal I/O are kept separate.

## 2. Problems It Solves

- Deadline-only sorting ignores whether the user can actually do a task right now (tired, noisy place, no internet)
- Task order dependencies are usually tracked manually in someone's head → wasted effort / blocked work
- No visibility into recurring avoidance patterns (tasks repeatedly postponed)
- Cloud productivity tools raise privacy concerns — Prioro is local-first

## 3. Core Features (Offline, Always Available)

- Add / edit / complete tasks — tagged with category, deadline, required energy level, required environment (quiet/noisy/internet)
- Task dependencies — "Task B can't start until Task A is done," with cycle detection (no impossible loops) and topological sort (valid task order)
- **"What should I do now?"** — quick check-in (energy/environment) → ranked task recommendations matching current state, skipping blocked/unready tasks
- Time-debt tracking — flags tasks rescheduled repeatedly
- Simple stats — completion streaks, time spent per category
- Local flat-file storage — no account, no cloud

## 4. AI Layer (Optional, Needs Internet)

- Natural-language task entry: `prioro add "gotta finish the report and call mom sometime this week"` → AI splits this into structured tasks
- Conversational suggestions: `prioro chat "I'm exhausted, what should I do?"` → human-sounding reply, not just a list
- AI-written weekly recap — patterns, encouragement, honest nudges
- AI suggests how to break big tasks into smaller steps

## 5. How Prioro "Knows" the User's State

No sensors — it's inferred, not detected:
1. **Direct check-in** — quick prompt each time: "How's your energy? [low/medium/high]" / "Where are you? [quiet/noisy]"
2. **Pattern learning** — over time, remembers past check-ins and can suggest a default (e.g. "usually low-energy after 9pm")
3. *(Future scope)* indirect signals like today's completion rate — weak hints only, not reliable alone

## 6. Sample CLI Commands (for reference)

```
prioro add "Write report" --category study --energy high --context quiet
prioro depend 5 --on 3
prioro now
prioro chat "I'm exhausted, what should I do?"
prioro complete 5
prioro stats
prioro plan --time 90
```

## 7. Tech Stack

- Language: C++17
- STL: vector, unordered_map, priority_queue, algorithm
- Storage: local flat file (upgradeable to SQLite later)
- AI connection: libcurl → LLM API (e.g. Claude or OpenAI), only when AI features are used
- Build tool: CMake
- Architecture: core logic as a separate static library; CLI is a thin consumer of it (so a future UI can reuse the same core without rewrites)

## 8. DSA Concepts Used

- Graphs (adjacency list) — modeling task dependencies
- DFS-based cycle detection — reject invalid circular dependencies
- Topological sort — compute valid task order
- Priority queue / heap — rank recommended tasks by score
- Hash maps — fast ID lookup, category grouping
- Custom sort comparators — multi-criteria sorting
- Greedy algorithm — priority-score based ranking
- (Stretch) Dynamic programming (0/1 knapsack) — optimal task selection within a free time budget (`prioro plan --time 90`)

## 9. Team & Responsibilities

| Designation | Responsibility |
|---|---|
| Team Leader | Data & Storage |
| Co-Leader | Dependency Graph |
| Member | Recommendation Engine |
| Member | AI Integration |
| Member | CLI & Testing |

## 10. Task for This Session: First Menu Page

Build the **first/main menu screen** of the Prioro terminal app — the screen a user sees right after launching the program, before doing anything else.

It should:
- Show the app name "Prioro" (and optionally a short tagline, e.g. "Your AI-powered activity partner")
- List the main menu options the user can choose from, numbered or lettered, matching the core commands above, e.g.:
  1. Add a task
  2. What should I do now?
  3. View all tasks
  4. Mark a task complete
  5. View stats
  6. Chat with Prioro (AI)
  7. Exit
- Accept user input to select an option (even if the actual logic behind each option isn't built yet — this screen can just show placeholder responses or loop back to the menu)
- Have a clean, simple terminal layout — doesn't need color/graphics, just clear spacing and a boxed/separated header, readable on a standard terminal
- Be written in C++17, compilable with a standard `g++`/CMake setup, as a single starting file (e.g. `main.cpp`) that the rest of the team can build on top of
