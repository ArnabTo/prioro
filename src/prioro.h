#ifndef PRIORO_H
#define PRIORO_H

#include <string>

const int MAX_TASKS = 100;
const int MAX_DEPS = 200;

// energy: 0=low 1=medium 2=high
// env: 0=quiet 1=noisy 2=internet
struct Task {
  int id;
  std::string title;
  std::string category;
  std::string deadline;
  int energy;
  int env;
  bool completed;
  int rescheduleCount;
};

struct UserState {
  int energy;
  int env;
};

void prioroLoad();
void prioroSave();

Task* prioroFindTask(int id);
int prioroTaskCount();
Task* prioroGetTaskByIndex(int index);

int prioroParseEnergy(const std::string& text);
int prioroParseEnv(const std::string& text);
const char* prioroEnergyName(int energy);
const char* prioroEnvName(int env);

int prioroAddTask(const Task& task, int dependsOnId, std::string& error);
bool prioroCompleteTask(int id, std::string& error);

// writes up to maxIds task ids into ids[], returns count
int prioroRecommend(UserState state, int ids[], int maxIds);

void prioroStats(int& total, int& completed, int& pending, int& highTimeDebt);
void prioroPrintCategoryStats();

#endif
