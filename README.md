# Custom Shell Implementation in C++

This project aims to create a custom shell in C++ that replicates standard Linux shell functionalities. It accepts user commands and arguments and executes them as subprocesses.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Phase A](#phase-a)
- [Phase B](#phase-b)
- [Phase C](#phase-c)
- [Run](#run)

## Features

- **Command Execution**: Accepts user commands and arguments, executes them as subprocesses using `fork` and `exec`.
- **Background Execution**: Supports running programs in the background using `&` suffix.
- **Environment Variable Interpretation**: Allows interpreting environment variables using `$` or `{}` syntax.
- **Command History**: Maintains a history of user-entered commands.
- **Redirection**: Handles input/output redirection (`<` and `>` operators) and command piping (`|` operator).

## Installation

Include instructions on how to install or set up the project here.

## Usage

Provide information on how to use the custom shell. Include examples and commands for better understanding.

## Contributing

Explain how others can contribute to the project, guidelines for pull requests, code formatting, etc.

## License

Specify the project's license and any additional information about licensing.

## Phase A

### Command Acceptance and Execution

- The shell accepts user commands and arguments.
- It utilizes the `fork` and `exec` functions to spawn a child process and execute the appropriate program.
- Search for programs in the paths specified by the `PATH` environment variable.
- Handles scenarios where programs are not found or found and executes them accordingly.
- Proper error handling for failed subprocess creation is implemented.
- Handles cases where users provide full paths to programs.

### Background Process Execution

- Supports executing programs in the background using the `&` suffix.
- Introduces a `myjobs` command to view all background processes along with their PID, command, and status.

## Phase B

### Environment Variable Interpretation

- Interprets environment variables using the prefix `$` or the curly braces `{}` syntax.
- Example: `echo $PATH` prints the value stored in the `PATH` variable.

### Command History Feature

- Maintains a history of user-entered commands, saving them to a text file (including invalid commands).
- Users can view the command history using the `myhistory` command in the shell.

## Phase C

### Redirection

#### Input/Output Redirection

- **Input Redirection (`<` Operator)**: Considers the argument following the `<` operator as the input file (STDIN) for the running program.
- **Output Redirection (`>` Operator)**: Considers the argument following the `>` operator as the output file (STDOUT) for the running program.

#### Command Piping

- **Pipe Operator (`|`)**: Creates a pipe between two programs surrounding the operator.
  - The preceding program writes its output to the PIPE.
  - The following program reads its input from the PIPE.
- **Multiple Piping**: Supports an unspecified number of pipes in a command, allowing chaining of more than one pipe in a single command.

## Run

### Compilation and Execution

```bash
g++ -Iinclude src/*.cpp -o out -std=c++20
./out
``
