#pragma once

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <fstream>

constexpr const char* filePath = "history.txt";

struct CmdData {
    std::string command;
    std::vector<std::string> args;
    bool isBackground;
};

class Shell {
public:
    void run();
    Shell();
    ~Shell();


private:
    std::vector<std::pair<pid_t, std::string>> m_bgProcesses;
    std::fstream m_historyFile;

    void addToHistory(const std::string&);
    void displayHistory();
    void prompt() const;
    void removeBgSign(CmdData&);
    void execute(const std::string&, CmdData&);
    std::vector<char*> setExecArgs(const CmdData&);
    CmdData parse(const std::string&);
    [[nodiscard]]std::string findCommand(const std::string&) const;
    static bool checkAccess(const std::string&);
    void doFork(CmdData&, const std::string&);
    void cd(const CmdData&);
    void myJobs();
    void updateArgIfEnvVarExists(std::vector<std::string>::iterator, const std::string&);
    void checkArgsAsEnvVar(CmdData&);
};
