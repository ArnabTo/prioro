#include "prioro.h"

#include <cctype>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

static Task g_tasks[MAX_TASKS];
static int g_taskCount = 0;
static int g_nextId = 1;

struct DepEdge {
  int dependent;
  int prerequisite;
};
static DepEdge g_deps[MAX_DEPS];
static int g_depCount = 0;

static void ensureDataDir() {
#ifdef _WIN32
  _mkdir("data");
#else
  mkdir("data", 0755);
#endif
  (void)errno;
}

static std::string toLower(std::string s) {
  for (size_t i = 0; i < s.size(); i++)
    s[i] = (char)std::tolower((unsigned char)s[i]);
  return s;
}

int prioroParseEnergy(const std::string& text) {
  std::string s = toLower(text);
  if (s == "low" || s == "l") return 0;
  if (s == "high" || s == "h") return 2;
  return 1;
}

int prioroParseEnv(const std::string& text) {
  std::string s = toLower(text);
  if (s == "noisy" || s == "n") return 1;
  if (s == "internet" || s == "i" || s == "net") return 2;
  return 0;
}

const char* prioroEnergyName(int energy) {
  if (energy == 0) return "low";
  if (energy == 2) return "high";
  return "medium";
}

const char* prioroEnvName(int env) {
  if (env == 1) return "noisy";
  if (env == 2) return "internet";
  return "quiet";
}

static bool energyFits(int required, int current) { return current >= required; }

static bool envFits(int required, int current) {
  if (required == 2) return current == 2;
  if (required == 0 && current == 1) return false;
  return true;
}

Task* prioroFindTask(int id) {
  for (int i = 0; i < g_taskCount; i++) {
    if (g_tasks[i].id == id) return &g_tasks[i];
  }
  return 0;
}

int prioroTaskCount() { return g_taskCount; }

Task* prioroGetTaskByIndex(int index) {
  if (index < 0 || index >= g_taskCount) return 0;
  return &g_tasks[index];
}

static bool taskComplete(int id) {
  Task* t = prioroFindTask(id);
  return t && t->completed;
}

static bool hasPrereq(int dependent, int prerequisite) {
  for (int i = 0; i < g_depCount; i++) {
    if (g_deps[i].dependent == dependent && g_deps[i].prerequisite == prerequisite)
      return true;
  }
  return false;
}

static bool reachesImpl(int from, int target, bool seen[1000]) {
  if (from == target) return true;
  if (from < 0 || from >= 1000 || seen[from]) return false;
  seen[from] = true;
  for (int i = 0; i < g_depCount; i++) {
    if (g_deps[i].dependent != from) continue;
    if (reachesImpl(g_deps[i].prerequisite, target, seen)) return true;
  }
  return false;
}

static bool wouldCycle(int dependent, int prerequisite) {
  bool seen[1000] = {false};
  return reachesImpl(prerequisite, dependent, seen);
}

static bool isBlocked(int taskId) {
  for (int i = 0; i < g_depCount; i++) {
    if (g_deps[i].dependent != taskId) continue;
    if (!taskComplete(g_deps[i].prerequisite)) return true;
  }
  return false;
}

static bool addDepEdge(int dependent, int prerequisite, std::string& error) {
  if (dependent == prerequisite) {
    error = "Task cannot depend on itself.";
    return false;
  }
  if (wouldCycle(dependent, prerequisite)) {
    error = "Dependency would create a cycle.";
    return false;
  }
  if (hasPrereq(dependent, prerequisite)) return true;
  if (g_depCount >= MAX_DEPS) {
    error = "Too many dependencies.";
    return false;
  }
  g_deps[g_depCount].dependent = dependent;
  g_deps[g_depCount].prerequisite = prerequisite;
  g_depCount++;
  return true;
}

static double scoreTask(const Task& t) {
  double score = t.rescheduleCount * 10.0;
  if (!t.deadline.empty()) score += 5.0;
  if (t.energy == 2) score += 1.0;
  return score;
}

void prioroLoad() {
  g_taskCount = 0;
  g_depCount = 0;
  g_nextId = 1;

  std::ifstream in("data/tasks.txt");
  if (in) {
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::stringstream ss(line);
      std::string part;
      std::string parts[8];
      int n = 0;
      while (std::getline(ss, part, '|') && n < 8) parts[n++] = part;
      if (n < 8 || g_taskCount >= MAX_TASKS) continue;
      Task& t = g_tasks[g_taskCount];
      t.id = std::stoi(parts[0]);
      t.title = parts[1];
      t.category = parts[2];
      t.deadline = parts[3];
      t.energy = std::stoi(parts[4]);
      t.env = std::stoi(parts[5]);
      t.completed = (parts[6] != "0");
      t.rescheduleCount = std::stoi(parts[7]);
      g_taskCount++;
      if (t.id >= g_nextId) g_nextId = t.id + 1;
    }
  }

  std::ifstream din("data/deps.txt");
  if (din) {
    std::string line;
    while (std::getline(din, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::stringstream ss(line);
      std::string a, b;
      if (!std::getline(ss, a, '|') || !std::getline(ss, b, '|')) continue;
      std::string err;
      addDepEdge(std::stoi(a), std::stoi(b), err);
    }
  }
}

void prioroSave() {
  ensureDataDir();
  std::ofstream out("data/tasks.txt");
  if (!out) return;
  out << "# id|title|category|deadline|energy|env|completed|reschedule\n";
  for (int i = 0; i < g_taskCount; i++) {
    Task& t = g_tasks[i];
    out << t.id << '|' << t.title << '|' << t.category << '|' << t.deadline << '|'
        << t.energy << '|' << t.env << '|' << (t.completed ? 1 : 0) << '|'
        << t.rescheduleCount << '\n';
  }

  std::ofstream dout("data/deps.txt");
  if (!dout) return;
  dout << "# dependent|prerequisite\n";
  for (int i = 0; i < g_depCount; i++)
    dout << g_deps[i].dependent << '|' << g_deps[i].prerequisite << '\n';
}

int prioroAddTask(const Task& task, int dependsOnId, std::string& error) {
  if (g_taskCount >= MAX_TASKS) {
    error = "Task list full.";
    return 0;
  }
  Task t = task;
  t.id = g_nextId++;
  g_tasks[g_taskCount] = t;
  g_taskCount++;

  if (dependsOnId > 0) {
    if (!prioroFindTask(dependsOnId)) {
      error = "Prerequisite task id not found.";
      g_taskCount--;
      g_nextId--;
      return 0;
    }
    if (!addDepEdge(t.id, dependsOnId, error)) {
      g_taskCount--;
      g_nextId--;
      return 0;
    }
  }
  prioroSave();
  return t.id;
}

bool prioroCompleteTask(int id, std::string& error) {
  Task* t = prioroFindTask(id);
  if (!t) {
    error = "Task id not found.";
    return false;
  }
  t->completed = true;
  prioroSave();
  return true;
}

static void sortScoresDesc(int ids[], double scores[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - 1 - i; j++) {
      if (scores[j] < scores[j + 1]) {
        double ts = scores[j];
        scores[j] = scores[j + 1];
        scores[j + 1] = ts;
        int ti = ids[j];
        ids[j] = ids[j + 1];
        ids[j + 1] = ti;
      }
    }
  }
}

int prioroRecommend(UserState state, int ids[], int maxIds) {
  int candIds[MAX_TASKS];
  double candScores[MAX_TASKS];
  int candCount = 0;

  for (int i = 0; i < g_taskCount; i++) {
    Task& t = g_tasks[i];
    if (t.completed) continue;
    if (!energyFits(t.energy, state.energy)) continue;
    if (!envFits(t.env, state.env)) continue;
    if (isBlocked(t.id)) continue;
    candIds[candCount] = t.id;
    candScores[candCount] = scoreTask(t);
    candCount++;
  }

  sortScoresDesc(candIds, candScores, candCount);

  int out = 0;
  for (int i = 0; i < candCount && out < maxIds; i++) ids[out++] = candIds[i];
  return out;
}

void prioroStats(int& total, int& completed, int& pending, int& highTimeDebt) {
  total = completed = pending = highTimeDebt = 0;
  for (int i = 0; i < g_taskCount; i++) {
    total++;
    if (g_tasks[i].completed)
      completed++;
    else
      pending++;
    if (g_tasks[i].rescheduleCount >= 3) highTimeDebt++;
  }
}

void prioroPrintCategoryStats() {
  for (int i = 0; i < g_taskCount; i++) {
    if (!g_tasks[i].completed) continue;
    std::string cat = g_tasks[i].category;
    if (cat.empty()) cat = "(none)";
    bool counted = false;
    for (int j = 0; j < i; j++) {
      if (g_tasks[j].completed && g_tasks[j].category == cat) {
        counted = true;
        break;
      }
    }
    if (counted) continue;
    int c = 0;
    for (int k = 0; k < g_taskCount; k++) {
      if (g_tasks[k].completed && g_tasks[k].category == cat) c++;
    }
    std::cout << "    " << cat << ": " << c << "\n";
  }
}
