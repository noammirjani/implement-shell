#include "InputParser.h"

InputParser& InputParser::getInstance() {
    static InputParser instance;
    return instance;
}

/**
 * @brief Parses the input line and returns a vector of CmdData objects
 *
 * @param line The input line
 * @return std::vector<CmdData>
 */
std::vector<CmdData> InputParser::parse(const std::string& line) {
    std::vector<CmdData> commands;
    m_iss.clear();
    m_iss.str(line);

    while (!m_iss.eof()) {
        commands.push_back(getCmdData());
    }

    return commands;
}

/**
 * @brief Returns a CmdData object
 *
 * @return CmdData
 */
CmdData InputParser::getCmdData() {

    CmdData data;
    m_iss >> data.command;

    std::string arg;
    while (m_iss >> arg && arg != "|") {
        if (arg == ">") {
           redirect(STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
        } else if (arg == "<") {
            redirect(STDIN_FILENO, O_RDONLY);
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

/**
 * @brief Removes the & sign from the last argument
 *
 * @param data The CmdData object
 */
void InputParser::removeBgSign(CmdData& data) {
    if (data.args.back() == "&") {
        // remove the last arg
        data.args.pop_back();
    } else {
        // remove the & from the last arg
        data.args.back().pop_back();
    }
}


/**
 * @brief Returns a vector of char* to be passed to execvp
 *
 * @param data The CmdData object
 * @return std::vector<char*>
 */
std::vector<char*> InputParser::setExecArgs(const CmdData& data) {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(data.command.c_str()));
    for (const auto& arg : data.args) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return args;
}


/**
 * @brief Checks if any of the arguments is an environment variable
 *
 * @param data The CmdData object
 */
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


/**
 * @brief Updates the argument if the environment variable exists
 *
 * @param arg The argument
 * @param var The environment variable
 */
void InputParser::updateArgIfEnvVarExists(std::vector<std::string>::iterator arg, const std::string& var) {
    char* value = getenv(var.c_str());
    if (value != nullptr) {
        *arg = value;
    }
}

/**
 * @brief Redirects the standard input or output to a file
 *
 * @param stdFd The standard input or output file descriptor
 * @param fMode The file mode
 */
void InputParser::redirect(unsigned int stdFd, int fMode) {
    int originalFd = dup(stdFd);
    if (originalFd == -1) {
        throw std::runtime_error("Failed to duplicate file descriptor");
    }
    std::string fName;
    if (!(m_iss >> fName)) {
        throw std::invalid_argument("Missing file name");
    }

    int fd = open(fName.c_str(), fMode, 0666);
    if (fd == -1) {
        throw std::invalid_argument("Failed to open file" + fName);
    }
    if (dup2(fd, stdFd) == -1) {
        throw std::runtime_error("Failed to redirect file");
    }
    if (close(fd) == -1) {
        throw std::runtime_error("Failed to close file");
    }

    // Save the original and redirected file descriptors for restoration
    m_fds_changes.push_back({originalFd, stdFd});
}


/**
 * @brief Restores the standard input and output file descriptors
 */
void InputParser::initFD() {
    for (const auto& pair : m_fds_changes) {
        if (dup2(pair.first, pair.second) == -1) {
            throw std::runtime_error("Failed to restore fd");
        }
        if (close(pair.first) == -1) {
            throw std::runtime_error("Failed to close fd");
        }
    }
    m_fds_changes.clear();
}
