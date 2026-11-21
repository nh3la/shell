SeaSh — A Mini Unix Shell in C
Project Overview
SeaSh is a lightweight command-line shell written in C to explore how Unix-like operating systems execute commands and manage processes. It provides an interactive terminal interface, supports running commands from a script file, and implements several familiar shell features. The project demonstrates low-level systems programming, command parsing, process creation, and file handling without relying on an existing shell to interpret each command.
How the Code Works
The main() function controls the shell's execution loop. It reads each line with fgets() and uses strtok() to split the input into a command and its arguments. When running interactively, print_prompt() displays the current username and working directory using getpwuid(), getuid(), and getcwd().

The program handles built-in commands directly:
- cd changes directories using chdir(), while cd - returns to the previous directory.
- history displays numbered past commands, and history -c clears the history file.
- exit closes the shell.
History-related functions save interactive commands to ~/.seash_history, read them back, and clear the file when requested.

For other commands, SeaSh calls fork() to create a child process and execvp() to run the requested program. The parent normally uses waitpid() to wait for the command to finish. Adding a standalone & at the end allows a command to start in the background without blocking the shell. Basic error messages use perror() and usage checks.

Running seash.c
gcc seash.c -o seash
./seash
