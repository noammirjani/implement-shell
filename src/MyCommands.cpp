#include "MyCommands.h"

MyCommands::MyCommands() {
    m_historyFile.open(filePath, std::ios::in | std::ios::out | std::ios::trunc);
}

MyCommands::~MyCommands() {
    m_historyFile.close();
}

/**
 * @brief Executes the command given by the user.
 * @param userCmd The command given by the user.
 */
void MyCommands::myHistory() {
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
 * @brief Adds the command given by the user to the history file.
 * @param userCmd The command given by the user.
 */
void MyCommands::addToHistory(const std::string& userCmd) {
    if (m_historyFile.is_open()) {
        m_historyFile << userCmd << std::endl;
    } else {
        throw std::runtime_error("File is not open.");
    }
}


/**
 * @brief Executes the command given by the user.
 * @param userCmd The command given by the user.
 */
void MyCommands::myJobs() {
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

    if (m_bgProcesses.empty()) {
        std::cout << "No background processes" << std::endl;
    }
}

void MyCommands::addBackgroundProcess(pid_t pid, const std::string& command) {
    m_bgProcesses.emplace_back(pid, command);
}
