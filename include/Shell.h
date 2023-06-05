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


struct CmdData {
    std::string command;
    std::vector<std::string> args;
    bool isBackground;
};

class Shell {
public:
    void run();

private:
    std::vector<std::pair<pid_t, std::string>> m_bgProcesses;


    void execute(const std::string&);
    std::vector<char*> setExecArgs(const CmdData&);
    CmdData parse(const std::string&);
    std::string findCommand(const std::string&) const;
    static bool checkAccess(const std::string&);

    void cd(const CmdData &data);
    void myJobs();
};



// the shell is divide to 2 parts - the parser: read input and enter  to command table
// the executor: execute the command table by creating a new process and run the command,
//               uses pipe to connect between processes
//
