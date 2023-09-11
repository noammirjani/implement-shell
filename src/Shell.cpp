#include "Shell.h"

void Shell::run() {
    std::string line;

    while (true) {
        try {
            prompt();
            std::getline(std::cin, line);
            std::vector<CmdData>  data = InputParser::getInstance().parse(line);

            if(data[0].command == "exit")
                break;

            if(data.size() > 1)
                executePipe(data);
            else execute(line, data[0]);

            InputParser::getInstance().initFD();
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

/**
 * Executes a single command.
 *
 * @param line The entire command line input.
 * @param data The command data.
 */
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


/**
 * Finds the full path of a command.
 *
 * @param command The command to find.
 * @return The full path of the command.
 * @throws std::invalid_argument If the command is not found.
 * @throws std::runtime_error If failed to retrieve the PATH environment variable.
 */
std::string Shell::findCommand(const std::string& command) {
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


/**
 * Checks if a command has executable access.
 *
 * @param command The command to check.
 * @return True if the command has executable access, false otherwise.
 */
bool Shell::checkAccess(const std::string& command) {
    return access(command.c_str(), X_OK) != -1;
}


/**
 * Executes a command using fork.
 *
 * @param data The command data.
 * @param line The entire command line input.
 * @throws std::runtime_error If fork failed or execvp failed.
 */
void Shell::doFork(CmdData& data, const std::string& line) {
    pid_t c_pid = fork();
    if (c_pid == -1) {
        throw std::runtime_error("fork failed");
    }
    else if (c_pid == 0) {          // child process
        executeChild(data);
    }
    else {                      // parent process
        if(!data.isBackground)
            wait(nullptr);
        else myCommands.addBackgroundProcess(c_pid, line);
    }
}

/**
 * Executes a command in the child process.
 *
 * @param data The command data.
 * @throws std::runtime_error If execvp failed.
 */
void Shell::cd(const CmdData& data) {
    // change directory - no need of fork
    if(data.args.empty())
        throw std::invalid_argument("cd: missing argument");
    if(chdir(data.args[0].c_str()) == -1)
        throw std::invalid_argument("cd: failed to change directory");
}


/**
 * Executes a command in the child process.
 *
 * @param data The command data.
 * @throws std::runtime_error If execvp failed.
 */
void Shell::executePipe(std::vector<CmdData>& commands) {
    size_t size = commands.size();
    std::vector<int> pipesFD(size * 2);

    for (size_t i = 0; i < size; i++) {
        if (pipe(pipesFD.data() + i * 2) == -1)
            throw std::runtime_error("pipe failed");
    }

    for (size_t i = 0; i < size; ++i) {
        pid_t c_pid = fork();
        if (c_pid == -1)
            throw std::runtime_error("fork failed");
        else if (c_pid == 0) {        // child process
            if (i != 0) { //first command - no need to read from pipe
                if (dup2(pipesFD[(i - 1) * 2], STDIN_FILENO) == -1)
                    throw std::runtime_error("dup2 failed");
            }
            if (i != size - 1) { //last command - no need to write to pipe
                if (dup2(pipesFD[i * 2 + 1], STDOUT_FILENO) == -1)
                    throw std::runtime_error("dup2 failed");
            }
            closePipes(pipesFD);
            executeChild(commands[i]);
        }
    }
    closePipes(pipesFD);
    for (size_t i = 0; i < size; i++) {
        wait(nullptr);
    }
}

/**
 * Closes all pipes.
 *
 * @param pipesFD The pipes file descriptors.
 */
void Shell::closePipes(std::vector<int>& pipesFD) {
    for (size_t i = 0; i < pipesFD.size(); i++) {
        close(pipesFD[i]);
    }
}


/**
 * Executes a command in the child process.
 *
 * @param data The command data.
 * @throws std::runtime_error If execvp failed.
 */
void Shell::executeChild(CmdData& data){
    std::string command = findCommand(data.command);
    std::vector<char*> args = InputParser::getInstance().setExecArgs(data);
    execvp(command.c_str(), args.data());
    throw std::runtime_error("execvp failed");
}