#pragma once
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

struct CmdData {
    std::string command;
    std::vector<std::string> args;
    bool isBackground;
};

// singleton class
class InputParser {
public:
    static InputParser& getInstance();
    std::vector<char*> setExecArgs(const CmdData&);
    void updateArgIfEnvVarExists(std::vector<std::string>::iterator, const std::string&);
    void checkArgsAsEnvVar(CmdData&);
    std::vector<CmdData> parse(const std::string&);
    void initFD();

private:
    std::istringstream m_iss;
    std::vector<std::pair<int, int>> m_fds_changes;


    InputParser() = default;
    InputParser(const InputParser&) = delete;
    InputParser& operator=(const InputParser&) = delete;

    void removeBgSign(CmdData&);
    void redirect(unsigned int, int);
    CmdData getCmdData();
};
