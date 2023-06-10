#include "InputParser.h"

InputParser& InputParser::getInstance() {
    static InputParser instance;
    return instance;
}

CmdData InputParser::parse(const std::string& line) {
    CmdData data;
    m_iss.clear();
    m_iss.str(line);

    m_iss >> data.command;

    std::string arg;
    while (m_iss >> arg) {
        if (arg == ">") {
            redirectOut();
        } else if (arg == "<") {
            redirectIn();
        } else if (arg == "|") {
            redirectPipe();
        }
        else {
            data.args.push_back(arg);
        }
    }

    data.isBackground = !data.args.empty() && data.args.back().ends_with("&");
    if (data.isBackground) {
        removeBgSign(data);
    }

    return data;
}

void InputParser::removeBgSign(CmdData& data) {
    if (data.args.back() == "&") {
        // remove the last arg
        data.args.pop_back();
    } else {
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
    char* value = getenv(var.c_str());
    if (value != nullptr) {
        *arg = value;
    }
}

void InputParser::redirectIn() {
    int originalStdin = dup(STDIN_FILENO);

    std::string inputFileName;
    if (!(m_iss >> inputFileName)) {
        throw std::runtime_error("Missing input file name after '<'");
    }

    int fd = open(inputFileName.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Failed to open input file");
    }
    if (dup2(fd, STDIN_FILENO) == -1) {
        throw std::runtime_error("Failed to redirect input");
    }
    if (close(fd) == -1) {
        throw std::runtime_error("Failed to close input file");
    }

    m_fds_changes.push_back({originalStdin, fd});
}

void InputParser::redirectOut() {
    int originalStdout = dup(STDOUT_FILENO);

    std::string outputFileName;
    if (!(m_iss >> outputFileName)) {
        throw std::invalid_argument("Missing output file name after '>'");
    }

    int fd = open(outputFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        throw std::runtime_error("Failed to open output file");
    }
    if (dup2(fd, STDOUT_FILENO) == -1) {
        throw std::runtime_error("Failed to redirect output");
    }

    m_fds_changes.push_back({originalStdout, fd});
}


void InputParser::initFD() {
    for (const auto& [originalFd, newFd] : m_fds_changes) {
        if (dup2(originalFd, newFd) == -1) {
            throw std::runtime_error("Failed to restore fd");
        }
        if (close(newFd) == -1) {
            throw std::runtime_error("Failed to close fd");
        }
    }
    m_fds_changes.clear();
}