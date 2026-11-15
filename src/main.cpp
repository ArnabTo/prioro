#include "prioro.h"

#include <iostream>
#include <limits>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

static void clearLine() {
  cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static string readLine(const char* prompt) {
  cout << prompt;
  string s;
  std::getline(cin, s);
  return s;
}

static int readInt(const char* prompt) {
  cout << prompt;
  int v = 0;
  cin >> v;
  clearLine();
  return v;
}

static void printHeader() {
  cout << "\n  +-------------------------------------+\n";
  cout << "  |              P R I O R O            |\n";
  cout << "  |   Your AI-powered activity partner  |\n";
  cout << "  +-------------------------------------+\n\n";
}

static void printMenu() {
  cout << "  1. Add a task\n";
  cout << "  2. What should I do now?\n";
  cout << "  3. View all tasks\n";
  cout << "  4. Mark a task complete\n";
  cout << "  5. View stats\n";
  cout << "  6. Chat with Prioro (AI)\n";
  cout << "  7. Exit\n\n";
  cout << "  Choice: ";
}

static void printTask(const Task& t) {
  cout << "  [" << t.id << "] " << t.title << (t.completed ? " (done)" : "") << "\n";
  cout << "      category: " << t.category << "  deadline: " << t.deadline << "\n";
  cout << "      energy: " << prioroEnergyName(t.energy) << "  context: "
       << prioroEnvName(t.env) << "  postponed: " << t.rescheduleCount << "\n";
}

static UserState promptCheckIn() {
  UserState s;
  s.energy = prioroParseEnergy(readLine("  Energy [low/medium/high]: "));
  s.env = prioroParseEnv(readLine("  Context [quiet/noisy/internet]: "));
  return s;
}

static void handleAdd() {
  Task t;
  t.title = readLine("  Title: ");
  if (t.title.empty()) {
    cout << "  Cancelled (empty title).\n";
    return;
  }
  t.category = readLine("  Category: ");
  t.deadline = readLine("  Deadline (text): ");
  t.energy = prioroParseEnergy(readLine("  Required energy [low/medium/high]: "));
  t.env = prioroParseEnv(readLine("  Required context [quiet/noisy/internet]: "));
  t.completed = false;
  t.rescheduleCount = 0;
  int dep = readInt("  Depends on task id (0 = none): ");
  string err;
  int id = prioroAddTask(t, dep, err);
  if (id <= 0)
    cout << "  Failed: " << err << "\n";
  else
    cout << "  Task added with id " << id << ".\n";
}

static void handleNow() {
  cout << "\n  Quick check-in\n";
  UserState state = promptCheckIn();
  int ids[10];
  int n = prioroRecommend(state, ids, 10);
  if (n == 0) {
    cout << "\n  No matching tasks (blocked, wrong state, or all done).\n";
    return;
  }
  cout << "\n  Suggested next:\n";
  for (int i = 0; i < n; i++) {
    Task* t = prioroFindTask(ids[i]);
    if (t) printTask(*t);
  }
}

static void handleView() {
  if (prioroTaskCount() == 0) {
    cout << "\n  No tasks yet.\n";
    return;
  }
  cout << "\n  All tasks:\n";
  for (int i = 0; i < prioroTaskCount(); i++) printTask(*prioroGetTaskByIndex(i));
}

static void handleComplete() {
  int id = readInt("  Task id to complete: ");
  string err;
  if (prioroCompleteTask(id, err))
    cout << "  Marked complete.\n";
  else
    cout << "  " << err << "\n";
}

static void handleStats() {
  int total, done, pending, debt;
  prioroStats(total, done, pending, debt);
  cout << "\n  Stats\n";
  cout << "  Total: " << total << "  Done: " << done << "  Pending: " << pending << "\n";
  cout << "  High time-debt (3+ postpones): " << debt << "\n";
  cout << "  Completed by category:\n";
  prioroPrintCategoryStats();
}

static void handleChat() {
  string msg = readLine("  Message: ");
  if (msg.empty()) return;
  cout << "\n  [AI offline stub] Use menu option 2 for offline suggestions.\n";
}

int main() {
  prioroLoad();

  bool running = true;
  while (running) {
    printHeader();
    printMenu();
    int choice = readInt("");
    cout << "\n";

    switch (choice) {
      case 1:
        handleAdd();
        break;
      case 2:
        handleNow();
        break;
      case 3:
        handleView();
        break;
      case 4:
        handleComplete();
        break;
      case 5:
        handleStats();
        break;
      case 6:
        handleChat();
        break;
      case 7:
        running = false;
        break;
      default:
        cout << "  Invalid choice.\n";
        break;
    }

    if (running) readLine("\n  Press Enter to return to menu...");
  }

  cout << "\n  Goodbye.\n";
  return 0;
}
