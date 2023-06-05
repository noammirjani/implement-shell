#include "Shell.h"

void Shell::run() {
    std::string line;

    while (true) {
        try {
            char* cwd = (char*)malloc(sizeof(char) * 1024);
            std::cout <<  getcwd(cwd, 1024) << "~$ ";
            std::getline(std::cin, line);
            if (line == "exit")break;
            execute(line);
            free (cwd);
        } catch (const std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        } catch (const std::exception& e) {
            perror(e.what());
            break;
        }
    }
}

CmdData Shell::parse(const std::string& line) {
    CmdData data;
    std::string arg;
    std::istringstream iss(line);

    iss >> data.command;

    while (iss >> arg) {
        data.args.push_back(arg);
    }

    data.isBackground = line[line.length() - 1] == '&';
    if(data.isBackground){
        // remove the & from the last arg
        int lastIndex = data.args.size() - 1;
        if(data.args[lastIndex].length() > 1) {
            data.args[lastIndex] = data.args[lastIndex].substr(0, data.args[lastIndex].length() - 1);
        }else
            data.args.pop_back();
    }

    return data;
}

std::vector<char*> Shell::setExecArgs(const CmdData& data) {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(data.command.c_str()));
    for (const auto& arg : data.args) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

void Shell::cd(const CmdData& data) {
    // change directory - no need of fork
    if(data.args.size() == 0)
        throw std::invalid_argument("cd: missing argument");
    if(chdir(data.args[0].c_str()) == -1)
        throw std::invalid_argument("cd: failed to change directory");
}

void Shell::myJobs() {

    std::cout << m_bgProcesses.size() << std::endl;
    if(m_bgProcesses.empty()){
        std::cout << "No background processes" << std::endl;
        return;
    }

    for (auto it = m_bgProcesses.begin(); it != m_bgProcesses.end();  ++it) {
        pid_t pid = it->first;
        int status;
        if (waitpid(pid, &status, WNOHANG) == pid) {
            m_bgProcesses.erase(it);
        } else {
            std::cout << "pid: " << pid << " cmd: " << it->second << " status: " << status << std::endl;
        }
    }
}



void Shell::execute(const std::string& line) {
    CmdData data = parse(line);

    if(data.command == "exit"){
        return;
    }

    if(data.command == "cd"){
        cd(data);
        return;
    }

    if(data.command == "myJobs"){
        myJobs();
        return;
    }

    pid_t c_pid = fork();
    if (c_pid == -1) {
        throw std::runtime_error("fork failed");
    } else if (c_pid == 0) { // child process
        if(data.isBackground) {
            std::cout << m_bgProcesses.size() << std::endl;
        }
        std::string command = findCommand(data.command);
        std::vector<char*> args = setExecArgs(data);
        execvp(command.c_str(), args.data());
        throw std::runtime_error("execvp failed");
    } else { // parent process
        if(!data.isBackground)
            wait(nullptr);
        else
            m_bgProcesses.push_back(std::make_pair(c_pid, line));
    }
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