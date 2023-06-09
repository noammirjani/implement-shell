#include "InputParser.h"

InputParser& InputParser::getInstance() {
    static InputParser instance;
    return instance;
}

CmdData InputParser::parse(const std::string& line) {
    CmdData data;
    std::istringstream iss(line);

    iss >> data.command;

    std::string arg;
    while (iss >> arg) {
        data.args.push_back(arg);
    }

    data.isBackground = !data.args.empty() && data.args.back().ends_with("&");
    if(data.isBackground){
        removeBgSign(data);
    }
    return data;
}

void InputParser::removeBgSign(CmdData& data) {
    if(data.args.back() == "&"){
        // remove the last arg
        data.args.pop_back();
    }
    else {
        // remove the & from the last arg
        data.args.back().pop_back();
    }
}

std::vector<char*> InputParser::setExecArgs(const CmdData& data) {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(data.command.c_str()));
    for (const auto& arg : data.args) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

void InputParser::checkArgsAsEnvVar(CmdData& data) {
    for (auto arg = data.args.begin(); arg != data.args.end(); ++arg) {
        if (arg->starts_with("${") && arg->ends_with("}")) {
            std::string var = arg->substr(2, arg->size() - 3);
            updateArgIfEnvVarExists(arg, var);
        } else if (arg->starts_with("$")) {
            updateArgIfEnvVarExists(arg, arg->substr(1));
        }
    }
}

void InputParser::updateArgIfEnvVarExists(std::vector<std::string>::iterator arg, const std::string& var) {
    char *value = getenv(var.c_str());
    if (value != nullptr) {
        *arg = value;
    }
}
