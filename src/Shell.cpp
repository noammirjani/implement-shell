#include "Shell.h"

void Shell::run() {
    std::string line;

    while (true) {
        try {
            prompt();
            std::getline(std::cin, line);
            CmdData data = InputParser::getInstance().parse(line);
            if(data.command == "exit")
                break;
            execute(line, data);
        }
        catch (const std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            // critical error
            perror(e.what());
            break;
        }
    }
}

void Shell::prompt() const {
    char cwd[PATH_MAX];
    std::cout << getcwd(cwd, PATH_MAX) << "~$ ";
}

void Shell::execute(const std::string& line, CmdData& data) {
    if (data.command.empty() || data.command == "exit")
        return;
    else if (data.command == "myhistory")
        myCommands.myHistory();
    else if (data.command == "myjobs")
        myCommands.myJobs();
    else {
        InputParser::getInstance().checkArgsAsEnvVar(data);
        if (data.command == "cd")
            cd(data);
        else
            doFork(data, line);
    }

    myCommands.addToHistory(line);
}

std::string Shell::findCommand(const std::string& command) const {
    std::string commandPath;

    if (checkAccess(command)) {
        return command;
    }

    std::string pathList(getenv("PATH"));
    if (pathList.empty()) {
        throw std::runtime_error("Failed to retrieve PATH environment variable");
    }

    std::istringstream iss(pathList);
    std::string path;

    while (std::getline(iss, path, ':')) {
        std::string fullPath(path + "/" + command);
        if (checkAccess(fullPath)) {
            return fullPath;
        }
    }

    throw std::invalid_argument("command not found");
}

bool Shell::checkAccess(const std::string& command) {
    return access(command.c_str(), X_OK) != -1;
}

void Shell::doFork(CmdData& data, const std::string& line) {
    pid_t c_pid = fork();
    if (c_pid == -1) {
        throw std::runtime_error("fork failed");
    }
    else if (c_pid == 0) {          // child process
        std::string command = findCommand(data.command);
        std::vector<char*> args = InputParser::getInstance().setExecArgs(data);
        execvp(command.c_str(), args.data());
        throw std::runtime_error("execvp failed");
    }
    else {                      // parent process
        if(!data.isBackground)
            wait(nullptr);
        else myCommands.addBackgroundProcess(c_pid, line);
    }
}

void Shell::cd(const CmdData& data) {
    // change directory - no need of fork
    if(data.args.empty())
        throw std::invalid_argument("cd: missing argument");
    if(chdir(data.args[0].c_str()) == -1)
        throw std::invalid_argument("cd: failed to change directory");
}

void Shell::checkArgsAsEnvVar(CmdData& data) {
    InputParser::getInstance().checkArgsAsEnvVar(data);
}
