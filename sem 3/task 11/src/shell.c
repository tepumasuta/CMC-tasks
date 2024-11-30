#include "shell.h"
#include "string_view.h"
#include "repl.h"

#ifndef __USE_POSIX2
#define __USE_POSIX2 1
#endif
#define __USE_POSIX 1

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

#define return_defer(RETVAL) do { result = (RETVAL); goto defer; } while (0)
#define error_return(ERRVALUE) do { *error = (ERRVALUE); goto on_error; } while(0)
#define error_exit_son() goto on_error_son\

static const char *builtins_list[] = { "cd", "type", "help", "exit" };
#define ARR_LEN(ARR) (sizeof((ARR))/sizeof(*(ARR)))

typedef void (*handler_t)(int);

handler_t default_sigint = SIG_DFL;
handler_t default_sigtstp = SIG_DFL;

volatile bool sigint_signal = false;
volatile bool sigtstp_signal = false;

static void shell_sigtstp_handler(int) {
    signal(SIGTSTP, shell_sigtstp_handler);
    sigtstp_signal = true;
}

static void setup_handlers(void) {
    default_sigint = signal(SIGINT, SIG_IGN);
    default_sigtstp = signal(SIGTSTP, shell_sigtstp_handler);
}

static void restore_handlers(void) {
    signal(SIGINT, default_sigint);
    signal(SIGTSTP, SIG_IGN);
}

static bool wait_exit_status_flags(struct Shell *shell, pid_t proc, bool save_exit_status, int flags, enum ShellError *error) {
    assert(shell);
    assert(error);
    int status = 0;
    errno = 0;
    while (waitpid(proc, &status, flags | WUNTRACED) < 0) {
        if (errno == EINTR) {
            if (sigtstp_signal) {
                if (shell->current_wainting == SOME_BIG_NUMBER) {
                    kill(proc, SIGCONT);
                } else {
                    kill(proc, SIGSTOP);
                    shell->waiting[shell->current_wainting++] = (struct BGProc){ .pid=proc, .stopped=true };
                    shell->last_exit_code = 0;
                    sigtstp_signal = false;
                    return true;
                }
            }
            if (sigint_signal) {
                printf("^C");
                sigint_signal = false;
            }
            errno = 0;
            continue;
        }
        return false;
    }
    if (WIFEXITED(status)) {
        if (save_exit_status) shell->last_exit_code = WEXITSTATUS(status);
        return true;
    }
    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        if (signal == SIGABRT) { shell->last_exit_code = 134; return true; }
        shell->last_exit_code = 1;
        return true;
    }
    assert(0 && "Unexpected");
    return false;
}

static bool wait_exit_status(struct Shell *shell, pid_t proc, bool save_exit_status, enum ShellError *error) {
    return wait_exit_status_flags(shell, proc, save_exit_status, 0, error);
}

bool shell_is_command_builtin(const char *name) {
    for (int i = 0; i < ARR_LEN(builtins_list); i++)
        if (!strcmp(name, builtins_list[i]))
            return true;
    return false;
}

enum CommandType {
    COMMAND_TYPE_NONE = 0,
    COMMAND_TYPE_BUILITN,
    COMMAND_TYPE_FILE,
};

static enum CommandType command_name_present(char *command_name, enum ShellError *error) {
    if (shell_is_command_builtin(command_name)) return COMMAND_TYPE_BUILITN;
    
    // Used inefficient, but simple way to find command
    // I should've forked and yara-yara, but it's not relevant i guess
    char *search_command = NULL;
    search_command = malloc(strlen(command_name) + 1 + 6 + 2);
    if (!search_command) error_return(SHELL_ERROR_FAILED_TO_ALLOC);
    strcpy(search_command, "which \"");
    strcat(search_command, command_name);
    strcat(search_command, "\"");
    FILE *f = popen(search_command, "r");
    int found = pclose(f);
    free(search_command);
    return !found ? COMMAND_TYPE_FILE : COMMAND_TYPE_NONE;
on_error:
    if (search_command) free(search_command);
    return COMMAND_TYPE_NONE;
}

static void normalize_path(char *path) {
    assert(path);
    assert(path[0] == '/');
    int length = strlen(path);
    int wi = 1;
    for (int i = 1; i < length;) {
        if (path[i] == '.' && path[i - 1] == '/') {
            if (i == length - 1) break;
            if (path[i + 1] == '/') {
                i += 2;
                continue;
            }
        }
        path[wi++] = path[i++];
    }
    length = wi;
    path[wi++] = '\0';
    wi = 0;
    for (int i = 0; i < length;) {
        if (path[i] == '/') {
            int j = i;
            while (j + 1 < length && path[j + 1] == '/') j++;
            i = j;
        }
        path[wi++] = path[i++];
    }
    length = wi ? wi : 1;
    if (wi && path[wi - 1] == '/' && length > 1) { path[wi - 1] = '\0'; }
    else { path[wi] = '\0'; }
    
    bool repeat = true;
    while (repeat) {
        repeat = false;
        {
            wi = 1;
            int i = 1;
            while (i + 2 < length && path[i] == '.' && path[i + 1] == '.' && path[i + 2] == '/')
                i += 3;
            if (i + 2 == length && path[i] == '.' && path[i + 1] == '.')
                i += 2;
            for (int j = i; j < length;)
                path[wi++] = path[j++];
            length = wi;
            path[wi] = '\0';
        }
        wi = 1;
        for (int i = 1; i < length;) {
            char *p1 = strchr(path + i, '/');
            if (!p1) {
                wi = length;
                break;
            }
            int j = (p1 - path) + 1;
            if (j + 1 < length && path[j] == '.' && path[j + 1] == '.') {
                if (j + 2 == length) {
                    repeat = true;
                    break;
                }
                if (path[j + 2] == '/') {
                    repeat = true;
                    i = j + 3;
                    while (i < length) path[wi++] = path[i++];
                    break;
                }
            }
            i = j;
            wi = j;
        }
        length = wi;
        path[wi++] = '\0';
        if (length > 1 && path[length - 1] == '/') path[length - 1] = '\0';
    }
}

static bool execute_builtins(struct Shell *shell, struct NodeBasicCommand *command, enum ShellError *error) {
    /* TODO: help */
    struct REPLSettings settings = { .colorized = true };
    if (shell->main) {
        settings.colorized = false;
    }
    assert(command);
    assert(command->name);
    assert(ARR_LEN(builtins_list) == 4);
    if (!strcmp(command->name, "exit")) exit(0);
    if (!strcmp(command->name, "type")) {
        int return_value = 0;
        char **arg = &command->args[1];
        while(*arg) {
            switch (command_name_present(*arg, error)) {
            case COMMAND_TYPE_BUILITN:
                printf("Builtin command `%s`\n", command->name);
                break;
            case COMMAND_TYPE_FILE:
                printf("A file command `%s` somewhere\n", command->name);
                break;
            case COMMAND_TYPE_NONE:
                repl_printf_error(settings, "No such command `%s`\n", command->name);
                if (!error != SHELL_ERROR_NONE) error_return(*error);
                return_value = 1;
            }
            arg++;
        }
        shell->last_exit_code = return_value;
        return true;
    }
    if (!strcmp(command->name, "cd")) {
        if (!command->args[1]) {
            repl_print_error(settings, "Too few arguments");
            error_return(SHELL_ERROR_CD_TOO_FEW_ARGUMENTS);
        }
        if (command->args[2]) {
            repl_print_error(settings, "Too many arguments");
            error_return(SHELL_ERROR_CD_TOO_MANY_ARGUMENTS);
        }
        char *new_path = shell->cwd;
        if (command->args[1][0] == '/') {
            new_path = malloc(strlen(command->args[1]) + 1);
            strcpy(new_path, command->args[1]);
            normalize_path(new_path);
        } else {
            new_path = malloc(strlen(shell->cwd) + strlen(command->args[1]) + 2);
            strcpy(new_path, shell->cwd);
            strcat(new_path, "/");
            strcat(new_path, command->args[1]);
            normalize_path(new_path);
        }
        errno = 0;
        if (chdir(new_path) < 0) {
            if (errno == ENAMETOOLONG) {
                free(new_path);
                repl_print_error(settings, "Too long name");
                error_return(SHELL_ERROR_CD_TOO_LONG_NAME);
            }
            repl_print_error(settings, "Failed to cd");
            error_return(SHELL_ERROR_GENERAL_SYSCALL);
        }
        strcpy(shell->cwd, new_path);
        free(new_path);
        shell->last_exit_code = 0;
        return true;
    }
    if (!strcmp(command->name, "help")) assert(0 && "TODO: help");
on_error:
    shell->last_exit_code = 1;
    return false;
}

static void close_or_exit(int fd) {
    assert(fd >= 0);
    errno = 0;
    while (close(fd) < 0) {
        if (errno == EINTR) continue;
        exit(EXIT_FAILURE);
    }
}

static bool close_signal_safe(int fd) {
    assert(fd >= 0);
    errno = 0;
    while (close(fd) < 0) {
        if (errno == EINTR) continue;
        return false;
    }
    return true;
}

static void close_or_ignore(int fd) {
    errno = 0;
    while (close(fd) < 0) {
        if (errno == EINTR) continue;
        return;
    }
}

static bool prepare_basic_io(const char *input, const char *output, bool append, enum ShellError *error) {
    assert(error);
    int out = -1;
    int in = -1;
    if (input) {
        in = open(input, O_RDONLY, 0440);
        if (in < 0) error_return(SHELL_ERROR_GENERAL_SYSCALL);
        if (dup2(in, 0) < 0) error_return(SHELL_ERROR_GENERAL_SYSCALL);
    }
    if (output) {
        out = open(output, O_WRONLY | (append ? O_APPEND : O_TRUNC) | O_CREAT, 0660);
        if (out < 0) error_return(SHELL_ERROR_GENERAL_SYSCALL);
        if (dup2(out, 1) < 0) error_return(SHELL_ERROR_GENERAL_SYSCALL);
    }
    return true;
on_error:
    if (out > 0) close_or_exit(out);
    if (in > 0) close_or_exit(in);
    return false;
}

static void try_add_color(struct Shell *shell, struct NodeBasicCommand *command) {
    if (!shell->main) return;
    if (strcmp(command->name, "grep") && strcmp(command->name, "ls")) return;
    int total = 0;
    while (command->args[total++]);
    for (int i = total; i > 1; i--)
        command->args[i] = command->args[i - 1];
    command->args[1] = "--color=auto";
}

static bool execute_basic_command(struct Shell *shell, struct NodeBasicCommand *command) {
    enum ShellError error = SHELL_ERROR_NONE;
    if (!execute_builtins(shell, command, &error) && error != SHELL_ERROR_NONE) error_exit_son();
    try_add_color(shell, command);
    int result = execvp(command->name, command->args);
    if (result < 0) error_exit_son();
    return false;
on_error_son:
    exit(CHILD_ERROR_CODE_BASIC_COMMAND);
}

static bool execute_single_basic_command(struct Shell *shell, struct NodeBasicCommand *command, enum ShellError *error) {
    assert(shell);
    assert(command);
    assert(error);
    if (shell_is_command_builtin(command->name)) {
        if (!execute_builtins(shell, command, error)) error_return(*error);
    } else {
        int exit_code_son = 0;
        pid_t p = fork();
        if (p < 0) error_return(SHELL_ERROR_FAILED_TO_FORK);
        else if (!p) {
            restore_handlers();
            if (!prepare_basic_io(command->input, command->output, command->append, error)) error_exit_son();
            if (!execute_basic_command(shell, command)) error_exit_son();
            exit(CHILD_ERROR_CODE_OK);
        } else {
            if (!wait_exit_status(shell, p, true, error)) error_return(*error);
        }
    }
    return true;
on_error:
    return false;
on_error_son:
    exit(CHILD_ERROR_CODE_PIPE_EXECUTE);
}

static bool execute_shell(struct Shell *shell, struct NodeShell *node, enum ShellError *error);

static bool execute_pipe(struct Shell *shell, struct NodePipeline *pipeline, enum ShellError *error) {
    assert(pipeline);
    if (pipeline->type == NODE_BASICITY_BASIC) {
        if (!prepare_basic_io(pipeline->command->input, pipeline->command->output,
                              pipeline->command->append, error)) error_return(*error);
        if (!execute_basic_command(shell, pipeline->command)) error_return(*error);
    } else {
        if (!execute_shell(shell, pipeline->shell, error)) error_return(*error);
    }
    return true;
on_error:
    return false;
}

static bool execute_pipeline(struct Shell *shell, struct NodePipeline *pipeline, int prev_pipe[2], enum ShellError *error) {
    assert(shell);
    assert(pipeline);
    assert(error);
    int new_pipe[2] = { -1, -1 };
    int exit_code_son = 0;
    if (pipeline->pipe && pipe(new_pipe) < 0) error_return(SHELL_ERROR_GENERAL_SYSCALL);
    pid_t p = fork();
    if (p < 0) error_return(SHELL_ERROR_FAILED_TO_FORK);
    else if (!p) {
        restore_handlers();
        if (prev_pipe) {
            if (dup2(prev_pipe[0], 0) < 0) error_exit_son();
            if (!close_signal_safe(prev_pipe[0])) error_exit_son();
            if (!close_signal_safe(prev_pipe[1])) error_exit_son();
        }
        if (pipeline->pipe) {
            if (dup2(new_pipe[1], 1) < 0) error_exit_son();
            if (!close_signal_safe(new_pipe[0])) error_exit_son();
            if (!close_signal_safe(new_pipe[1])) error_exit_son();
        }
        if (!execute_pipe(shell, pipeline, error)) error_return(*error);
        exit(CHILD_ERROR_CODE_OK);
    } else {
        if (prev_pipe) {
            if (!close_signal_safe(prev_pipe[0])) error_return(SHELL_ERROR_FAILED_TO_CLOSE);
            if (!close_signal_safe(prev_pipe[1])) error_return(SHELL_ERROR_FAILED_TO_CLOSE);
        }
        if (pipeline->pipe) {
            if (!execute_pipeline(shell, pipeline->pipe, new_pipe, error)) error_return(*error);
            if (!wait_exit_status(shell, p, false, error)) error_return(*error);
            return true;
        }
        if (!wait_exit_status(shell, p, true, error)) error_return(*error);
        return true;
    }
on_error:
    if (new_pipe[0] >= 0) close_or_exit(new_pipe[0]);
    if (new_pipe[1] >= 0) close_or_exit(new_pipe[1]);
    return false;
on_error_son:
    if (pipeline->pipe) {
        close_or_ignore(new_pipe[0]);
        close_or_ignore(new_pipe[1]);
    }
    if (prev_pipe) {
        close_or_ignore(prev_pipe[0]);
        close_or_ignore(prev_pipe[1]);
    }
    exit(CHILD_ERROR_CODE_PIPE_EXECUTE);
}

static bool execute_node_conditional(struct Shell *shell, struct NodeConditional *conditional, enum ShellError *error) {
    assert(shell);
    assert(conditional);
    assert(error);
    if (conditional->many) {
        if (!execute_pipeline(shell, conditional->commands, NULL, error)) error_return(*error);
    } else {
        if (!execute_single_basic_command(shell, conditional->command, error)) error_return(*error);
    }
    if (conditional->next) {
        if (conditional->if_false && shell->last_exit_code
            || !conditional->if_false && !shell->last_exit_code
        ) {
            if (!execute_node_conditional(shell, conditional->next, error)) error_return(*error);
            return true;
        }
    }
    return true;
on_error:
    return false;
}

static pid_t go_background(struct Shell *shell, enum ShellError *error) {
    if (shell->current_wainting == SOME_BIG_NUMBER) error_return(SHELL_ERROR_TOO_MANY_BG);
    int blackhole = -1;
    pid_t child = fork();
    if (child < 0) error_return(SHELL_ERROR_FAILED_TO_FORK);
    else if (!child) {
        restore_handlers();
        if (setpgid(0, 0) < 0) { assert(0 && "Unexpected"); abort(); }
        blackhole = open("/dev/null", O_RDWR, 0600);
        if (blackhole < 0) return child; /* Doesn't care, only for aesthetic */
        if (dup2(blackhole, 0) < 0) error_exit_son(); /* Doesn't care, only about fd closing */
        return child;
    } else {
        shell->waiting[shell->current_wainting++] = (struct BGProc){ .pid=child, .stopped=false };
        return child;
    }
on_error:
    return -1;
on_error_son:
    if (blackhole >= 0) close_or_ignore(blackhole);
    return child;
}

static bool execute_shell(struct Shell *shell, struct NodeShell *node, enum ShellError *error) {
    pid_t son = getpid();
    if (node->bg) {
        if ((son = go_background(shell, error)) < 0) error_return(*error);
        if (!son) {
            if (!prepare_basic_io(node->input, node->output, node->append, error)) error_return(*error);
            shell->main = false;
            if (!execute_node_conditional(shell, node->command, error)) error_exit_son();
            exit(CHILD_ERROR_CODE_OK);
        } else {
            if (node->next) return execute_shell(shell, node->next, error);
            return true;
        }
    }
    if (!prepare_basic_io(node->input, node->output, node->append, error)) error_return(*error);
    if (node->new_shell) {
        pid_t p = fork();
        if (p < 0) error_return(SHELL_ERROR_FAILED_TO_FORK);
        else if (!p) {
            restore_handlers();
            shell->main = false;
            if (!execute_node_conditional(shell, node->command, error)) error_exit_son();
            exit(CHILD_ERROR_CODE_OK);
        } else {
            if (!wait_exit_status(shell, p, true, error)) error_return(*error);
        }
    } else {
        if (!execute_node_conditional(shell, node->command, error)) error_return(*error);
    }
    if (node->next) return execute_shell(shell, node->next, error);
    return true;
on_error:
    return false;
on_error_son:
    exit(CHILD_ERROR_CODE_CONDITIONAL);
}

bool shell_execute(struct Shell *shell, struct AST *ast, enum ShellError *error) {
    assert(shell);
    assert(ast);
    assert(error);
    return execute_shell(shell, ast->root, error);
}

struct Shell *shell_create(enum ShellError *error) {
    setup_handlers();
    struct Shell *shell = NULL;
    shell = malloc(sizeof(*shell));
    if (!shell) error_return(SHELL_ERROR_FAILED_TO_ALLOC);
    errno = 0;
    if (getlogin_r(shell->login, LOGIN_MAX_LEN)) {
        if (errno == ERANGE) error_return(SHELL_ERROR_LOGIN_CAP);
        error_return(SHELL_ERROR_GENERAL_SYSCALL);
    }
    shell->login[LOGIN_MAX_LEN] = '\0'; /* For safety reasons */
    if (!getcwd(shell->cwd, PATH_MAX)) {
        if (errno == ERANGE) { assert(0 && "Path greater than PATH_MAX"); }
        else { error_return(SHELL_ERROR_GENERAL_SYSCALL); }
    }
    shell->cwd[PATH_MAX] = '\0'; /* For safety reasons */
    shell->last_exit_code = 0;
    shell->current_wainting = 0;
    shell->main = true;
    return shell;
on_error:
    if (shell) free(shell);
    return NULL;
}

struct Shell *shell_copy(struct Shell *shell, enum ShellError *error) {
    struct Shell *new_shell = NULL;
    new_shell = malloc(sizeof(*shell));
    if (!new_shell) error_return(SHELL_ERROR_FAILED_TO_ALLOC);
    memcpy(new_shell->cwd, shell->cwd, PATH_MAX + 1);
    memcpy(new_shell->login, shell->login, LOGIN_MAX_LEN + 1);
    new_shell->last_exit_code = shell->last_exit_code;
    return new_shell;
on_error:
    if (new_shell) free(new_shell);
    return NULL;
}

void shell_destroy(struct Shell *shell) {
    if (shell) {
        shell_invoke(shell, false);
        free(shell);
    }
}

void shell_invoke(struct Shell *shell, bool log) {
    int wi = 0;
    int real_exit = shell->last_exit_code;
    enum ShellError error;
    for (int i = 0; i < shell->current_wainting; i++) {
        if (!shell->waiting[i].stopped
            && wait_exit_status_flags(shell, shell->waiting[i].pid, true, WNOHANG, &error)) {
            if (shell->main && log) printf("Completed [%d] %d with exit code %d\n", i, shell->waiting[i].pid, shell->last_exit_code);
            continue;
        }
        shell->waiting[wi++] = shell->waiting[i];
    }
    shell->last_exit_code = real_exit;
    shell->current_wainting = wi;
}
