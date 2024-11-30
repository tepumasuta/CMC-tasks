#ifndef SHELL_H
#define SHELL_H 1

#include "ast.h"

#ifdef __linux
#include <linux/limits.h>
#else
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#endif
#include <sys/types.h>

#define SOME_BIG_NUMBER 8192
#define LOGIN_MAX_LEN 32

struct BGProc {
    pid_t pid;
    bool stopped;
};

struct Shell {
    bool main;
    char cwd[PATH_MAX + 1];
    char login[LOGIN_MAX_LEN + 1];
    int last_exit_code;
    struct BGProc waiting[SOME_BIG_NUMBER];
    int current_wainting;
};

enum ShellError {
    SHELL_ERROR_NONE = 0,
    SHELL_ERROR_FAILED_TO_ALLOC,
    SHELL_ERROR_LOGIN_CAP,
    SHELL_ERROR_GENERAL_SYSCALL,
    SHELL_ERROR_FAILED_TO_CLOSE,
    SHELL_ERROR_FAILED_TO_FORK,
    SHELL_ERROR_TOO_MANY_BG,
    SHELL_ERROR_CD_TOO_LONG_NAME,
    SHELL_ERROR_CD_TOO_MANY_ARGUMENTS,
    SHELL_ERROR_CD_TOO_FEW_ARGUMENTS,
};

enum ChildErrorCode {
    CHILD_ERROR_CODE_OK = 0,
    CHILD_ERROR_CODE_PIPE_EXECUTE,
    CHILD_ERROR_CODE_CONDITIONAL,
    CHILD_ERROR_CODE_BASIC_COMMAND,
    CHILD_ERROR_CODE_BACKGROUND,
};

bool shell_execute(struct Shell *shell, struct AST *ast, enum ShellError *error);
bool shell_is_command_builtin(const char *name);
void shell_invoke(struct Shell *shell, bool log);

struct Shell *shell_create(enum ShellError *error);
struct Shell *shell_copy(struct Shell *shell, enum ShellError *error);
void shell_destroy(struct Shell *shell);

extern volatile bool sigint_signal;
extern volatile bool sigtstp_signal;

#endif
