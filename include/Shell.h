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
    MyCommands myCommands;

    void prompt() const;
    //commands handler
    static std::string findCommand(const std::string&);
    static bool checkAccess(const std::string&);
    static void cd(const CmdData&);
    //pipes
    void executePipe(std::vector<CmdData>&);
    static void closePipes(std::vector<int>&);
    // Added function declarations
    static void executeChild(CmdData&);

    // regular execute
    void doFork(CmdData&, const std::string&);
    void execute(const std::string&, CmdData&);
};
