#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX_LINE 1024
#define MAX_ARGS 128

void print_prompt() {
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) {
            printf("%s@🐚 %s$ ", pw->pw_name, cwd);
        }
    }
}

void get_history_file(char *path, size_t size) {
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL) {
        snprintf(path, size, "%s/.seash_history", pw->pw_dir);
    }
}

void append_history(char *line) {
    char history_path[MAX_LINE];
    get_history_file(history_path, sizeof(history_path));
    
    FILE *f = fopen(history_path, "a");
    if (!f) {
        perror("open");
        exit(1);
    }
    
    int is_empty = 1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
            is_empty = 0;
            break;
        }
    }
    
    if (!is_empty) {
        char temp[MAX_LINE];
        strcpy(temp, line);
        
        char *token = strtok(temp, " \t\n");
        if (token != NULL) {
            fprintf(f, "%s", token);
            token = strtok(NULL, " \t\n");
            while (token != NULL) {
                fprintf(f, " %s", token);
                token = strtok(NULL, " \t\n");
            }
            fprintf(f, "\n");
        }
    }
    fclose(f);
}

void print_history() {
    char history_path[MAX_LINE];
    get_history_file(history_path, sizeof(history_path));
    
    FILE *f = fopen(history_path, "r");
    if (!f) {
        return;
    }
    
    char line[MAX_LINE];
    int counter = 1;
    while (fgets(line, sizeof(line), f) != NULL) {
        printf("%d %s", counter++, line);
    }
    fclose(f);
}

void clear_history() {
    char history_path[MAX_LINE];
    get_history_file(history_path, sizeof(history_path));
    
    FILE *f = fopen(history_path, "w");
    if (f) {
        fclose(f);
    }
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int interactive = 1;
    
    if (argc > 2) {
        fprintf(stderr, "Usage: ./seash [script]\n");
        exit(1);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror(argv[1]);
            exit(1);
        }
        interactive = 0;
    }
    
    char line[MAX_LINE];
    char previous_dir[MAX_LINE] = "";
    
    while (1) {
        if (interactive) {
            print_prompt();
            fflush(stdout);
        }
        
        if (fgets(line, sizeof(line), input) == NULL) {
            break; 
        }
        
        if (interactive) {
            append_history(line);
        }
        
        char *args[MAX_ARGS];
        int i = 0;
        char *token = strtok(line, " \t\n");
        
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[i] = NULL;
        
        if (i == 0) {
            continue;
        }
        
        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[i-1] = NULL;
            i--;
            if (i == 0) continue;
        }
        
        if (strcmp(args[0], "exit") == 0) {
            if (i != 1) {
                fprintf(stderr, "seash: Usage error\n");
            } else {
                exit(EXIT_SUCCESS);
            }
        } else if (strcmp(args[0], "cd") == 0) {
            if (i != 2) {
                fprintf(stderr, "seash: Usage error\n");
            } else {
                if (strcmp(args[1], "-") == 0) {
                    if (strlen(previous_dir) > 0) {
                        char cwd[MAX_LINE];
                        getcwd(cwd, sizeof(cwd));
                        if (chdir(previous_dir) != 0) {
                            perror("cd");
                        } else {
                            printf("%s\n", previous_dir);
                            strcpy(previous_dir, cwd);
                        }
                    } else {
                        char cwd[MAX_LINE];
                        getcwd(cwd, sizeof(cwd));
                        printf("%s\n", cwd);
                    }
                } else {
                    char cwd[MAX_LINE];
                    getcwd(cwd, sizeof(cwd));
                    if (chdir(args[1]) != 0) {
                        perror("cd");
                    } else {
                        strcpy(previous_dir, cwd);
                    }
                }
            }
        } else if (strcmp(args[0], "history") == 0) {
            if (i == 1) {
                print_history();
            } else if (i == 2 && strcmp(args[1], "-c") == 0) {
                clear_history();
            } else {
                fprintf(stderr, "seash: Usage error\n");
            }
        } else {
            pid_t pid = fork();
            
            if (pid < 0) {
                perror("fork");
            } else if (pid == 0) {
                execvp(args[0], args);
                perror(args[0]);
                exit(EXIT_FAILURE);
            } else {
                if (!background) {
                    waitpid(pid, NULL, 0);
                }
            }
        }
    }
    
    if (!interactive) {
        fclose(input);
    }
    
    exit(EXIT_SUCCESS);
}
