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
#include "InputParser.h"
#include "MyCommands.h"

class Shell {
public:
    void run();

private:
    void prompt() const;
    void execute(const std::string&, CmdData&);
    std::string findCommand(const std::string&) const;
    static bool checkAccess(const std::string&);
    void doFork(CmdData&, const std::string&);
    void cd(const CmdData&);
    void checkArgsAsEnvVar(CmdData&);

    MyCommands myCommands;
};

