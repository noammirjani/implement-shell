# Custom Shell Implementation in C++

This project involves the creation of a custom shell in C++ that simulates the standard Linux shell.
It accepts user commands and arguments, and then executes them as subprocesses.

## Phase A:
1. **Command Acceptance and Execution**:
    * The shell will accept commands and arguments from the user.
    * It utilizes the `fork` and `exec` family of functions to spawn a child process and run the appropriate program.
    * The shell will search for the program in the paths specified by the `PATH` environment variable.
        * If the program is not found, an appropriate message will be displayed.
        * If the program is found, the shell will execute it as a child process and wait for its completion.
        * Proper error handling should be implemented for failed subprocess creation.
    * The shell should also handle cases where a user provides a full path to the program, e.g., `/bin/echo hello`.
    * Arguments provided by the user will be forwarded to the program, e.g., for the input `echo hello world`, the program is `echo` and the arguments are `hello` and `world`.

2. **Background Process Execution**:
    * The shell supports executing programs in the background if the command is suffixed with `&`.
    * A `myjobs` command has been added to the shell allowing users to view all background processes along with details like PID, command, and status.

## Phase B:
3. **Environment Variable Interpretation**:
    * The shell can interpret environment variables using the prefix `$` or the curly braces `{}` syntax. For example, entering `echo $PATH` will print the value stored in the `PATH` variable.

4. **Command History Feature**:
    * The shell maintains a history of the commands entered by the user. This history is saved to a text file, appending all user inputs (including invalid commands).
    * Users can view the command history by executing the `myhistory` command in the shell.

## Phase C:
### Redirection:

1. **Input/Output Redirection**:
    * **Input Redirection (`<` Operator)**: If a command contains the `<` operator, the argument following this operator is considered as the input file (STDIN) for the running program.
    * **Output Redirection (`>` Operator)**: If a command contains the `>` operator, the argument following this operator is considered as the output file (STDOUT) for the running program.

### Pipe:

2. **Command Piping**:
    * **Pipe Operator (`|`)**: This operator allows the user to create a pipe between two programs surrounding the operator.
        * The program preceding the `|` operator writes its output to the PIPE.
        * The program following the `|` operator reads its input from the PIPE.
    * **Multiple Piping**: The shell supports an unspecified number of pipes in a command, meaning users can chain more than one pipe in a single command!

## run
#bash
```
g++ -Iinclude src/*.cpp -o out -std=c++20
./out
```
