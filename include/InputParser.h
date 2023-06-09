#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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
    CmdData parse(const std::string&);

private:
    InputParser() = default;
    InputParser(const InputParser&) = delete;
    InputParser& operator=(const InputParser&) = delete;

    void removeBgSign(CmdData&);
};
