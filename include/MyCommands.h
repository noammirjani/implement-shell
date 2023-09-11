#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdexcept>

constexpr const char* filePath = "history.txt";

class MyCommands {
public:
    MyCommands();
    ~MyCommands();

    void myHistory();
    void addToHistory(const std::string&);

    void myJobs();
    void addBackgroundProcess(pid_t pid, const std::string& command);

private:
    std::vector<std::pair<pid_t, std::string>> m_bgProcesses;
    std::fstream m_historyFile;
};

