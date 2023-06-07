#include "Shell.h"

Shell::Shell(){
    m_historyFile.open(filePath, std::ios::in | std::ios::out | std::ios::trunc);
}

Shell::~Shell() {
    m_historyFile.close();
}

/**
 * Run the shell application.
 */
void Shell::run() {
    std::string line;

    while (true) {
        try {
            prompt();
            std::getline(std::cin, line);
            CmdData data = parse(line);
            if(data.command == "exit")break;
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

/**
 * Execute a command.
 * @param line The command line input.
 */
void Shell::execute(const std::string& line, CmdData& data) {
    if(data.command.empty() || data.command == "exit")
        return;

    else if(data.command == "myhistory")
        displayHistory();

    else if(data.command == "myjobs")
        myJobs();

    else{
        checkArgsAsEnvVar(data);
        if(data.command == "cd")      // change directory - no need of fork
            cd(data);
        else doFork(data, line);
    }

    addToHistory(line);
}


/** Display the shell prompt.*/
void Shell::prompt() const {
    char cwd[PATH_MAX];
    std::cout <<  getcwd(cwd, PATH_MAX) << "~$ ";
}

/**
 * Parse the command line input.
 *
 * @param line The command line input.
 * @return The parsed command data.
 */
CmdData Shell::parse(const std::string& line) {
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


/**
 * Remove the background sign from the command data.
 * @param data The command data.
 */
void Shell::removeBgSign(CmdData& data) {

    if(data.args.back() == "&"){
        // remove the last arg
        data.args.pop_back();
    }
    else {
        // remove the & from the last arg
        data.args.back().pop_back();
    }
}



/**
 * Set the arguments for executing a command.
 * @param data The command data.
 * @return The arguments for execution.
 */
std::vector<char*> Shell::setExecArgs(const CmdData& data) {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(data.command.c_str()));
    for (const auto& arg : data.args) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return args;
}


/**
 * Change the current directory.
 * @param data The command data containing the directory argument.
 */
void Shell::cd(const CmdData& data) {
    // change directory - no need of fork
    if(data.args.empty())
        throw std::invalid_argument("cd: missing argument");
    if(chdir(data.args[0].c_str()) == -1)
        throw std::invalid_argument("cd: failed to change directory");
}


/** Display information about background processes. */
void Shell::myJobs() {

    // check if any background process has finished
    for (auto it = m_bgProcesses.begin(); it != m_bgProcesses.end();) {
        pid_t pid = it->first;
        int status;
        if (waitpid(pid, &status, WNOHANG) == pid) {
            m_bgProcesses.erase(it); //erase returns the next iterator
        } else {
            std::cout << "pid: " << pid << " || cmd: " << it->second << " || status: " << WEXITSTATUS(status)  << "running" << std::endl;
            ++it;
        }
    }

    // if all the processes have finished the vector is empty - print message
    if(m_bgProcesses.empty()) {
        std::cout << "No background processes" << std::endl;
    }
}



/**
 * Add a command to the history.
 * @param line The command line input.
 */
void Shell::checkArgsAsEnvVar(CmdData& data) {
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
 * Add a command to the history.
 * @param line The command line input.
 */
void Shell::updateArgIfEnvVarExists(std::vector<std::string>::iterator arg, const std::string& var) {
    char *value = getenv(var.c_str());
    if (value != nullptr) {
        *arg = value;
    }
}

/**
 * Fork a new process and execute the command.
 *
 * @param data The command data.
 * @param line The command line input.
 */
void Shell::doFork(CmdData& data, const std::string& line){
    pid_t c_pid = fork();
    if (c_pid == -1) {
        throw std::runtime_error("fork failed");
    }
    else if (c_pid == 0) {          // child process
        std::string command = findCommand(data.command);
        std::vector<char*> args = setExecArgs(data);
        execvp(command.c_str(), args.data());
        throw std::runtime_error("execvp failed");
    }
    else {                      // parent process
        if(!data.isBackground)
            wait(nullptr);
        else m_bgProcesses.emplace_back(c_pid, line);
    }
}


/**
 * Find the command executable in the PATH environment variable.
 * @param command The command name.
 * @return The full path of the command.
 */
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

/**
 * Check if the command is executable.
 * @param command The command name.
 * @return True if the command is executable, false otherwise.
 */
bool Shell::checkAccess(const std::string& command) {
    return access(command.c_str(), X_OK) != -1;
}

/**
 * Display the history of commands.
 */
void Shell::displayHistory() {
    if (m_historyFile.is_open()) {
        m_historyFile.seekg(0, std::ios::beg);
        std::string line;
        while (std::getline(m_historyFile, line)) {
            std::cout << line << std::endl;
        }
        m_historyFile.clear();
    } else {
        throw std::runtime_error("File is not open.");
    }
}


/**
 * Add a command to the history file.
 * @param data The command data.
 */
void Shell::addToHistory(const std::string& userCmd) {

    if (m_historyFile.is_open()) {
        m_historyFile << userCmd << std::endl;
    } else {
        throw std::runtime_error("File is not open.");
    }
}