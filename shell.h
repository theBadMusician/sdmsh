#ifndef SHELL_H
#define SHELL_H

#include <stdio.h> /* FILE* */
#include <sys/queue.h>

#define SHELL_EOF -256

#define ARGS_RANGE(range_expr) do {                            \
    if (!(range_expr)) {                                       \
        fprintf(stderr                                         \
                ,"%s: argument number must be in range '%s'\n" \
                , argv[0], #range_expr);                       \
        return -1;                                             \
    }                                                          \
} while(0)

#define ARG_LONG(name, str_val, out, range_expr) do {             \
    long arg;                                                     \
    errno = 0;                                                    \
    arg = strtol(str_val, NULL, 0);                               \
    if ((errno == ERANGE && (arg == LONG_MAX || arg == LONG_MIN)) \
        || (errno != 0 && arg == 0)) {                            \
        fprintf(stderr, name": must be a digit\n");               \
        return -1;                                                \
    }                                                             \
    if (!(range_expr)) {                                          \
        fprintf(stderr, name": must be in range '%s'\n"           \
                , #range_expr);                                   \
        return -1;                                                \
    }                                                             \
    out = arg;                                                    \
} while(0)

#define SF_SCRIPT_MODE      (1 << 1)

#define SCF_NONE            0
#define SCF_USE_DRIVER      (1 << 0)
#define SCF_NO_HISTORY      (1 << 1)
#define SCF_DRIVER_FILENAME (1 << 2)
#define SCF_DRIVER_NET      (1 << 3)

struct shell_config;
typedef int (*command_handler)(struct shell_config *sc, char *argv[], int argc);

struct commands_t {
    char *name;
    command_handler handler;
    unsigned long flags;
    char *usage;
    char *help;
};

struct driver_t {
    char *name;
    unsigned long flags;
    char *usage;
    char *help;
};

enum {
    SHELL_INPUT_TYPE_FILE = 1,
    SHELL_INPUT_TYPE_ARGV = 2
};

struct shell_input;
struct shell_input {
    STAILQ_ENTRY(shell_input) next_input;

    int type;
    FILE *input;
    union {
        char *script_file;
        struct {
            char *script_string;
            char *pos; /* current char in argument */
        };
    };
};

#define SHELL_MAX_INPUT 0xff
struct shell_config {
    /* data need to be filled by user */
    char *progname;
    FILE *input;
    char *prompt;
    void *cookie;  /** point to custom data for commands */
    unsigned int flags;
    STAILQ_HEAD(inputs_list, shell_input) inputs_list;
    int inputs_count;
    struct commands_t *commands;
    struct driver_t *drivers;

    /* internal data */
    int   shell_quit;
    char *shell_input;
    char *history_file;
};

void rl_callback(char* line);
void shell_init(struct shell_config *sc);
void shell_init_input_argv(struct shell_config *sc, int argc, char **argv);
void shell_deinit(struct shell_config *sc);
int  shell_handle(struct shell_config *sc);
void shell_show_help(struct shell_config *sc, char *name);

void shell_forced_update_display(struct shell_config *sc);

void shell_input_init(struct shell_config *sc);
void shell_input_init_current(struct shell_config *sc);
int  shell_input_add(struct shell_config *sc, int type, ...);
struct shell_input* shell_input_next(struct shell_config *sc);

#endif
