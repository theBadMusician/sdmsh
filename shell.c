/* for asprintf() */
#define _GNU_SOURCE

#include <assert.h>
#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h> /* strdup() */

#include <readline/readline.h>

#include <shell.h>
#include <shell_completion.h>
#include <shell_history.h>
#include <shell_help.h>
#include <utils.h>

int shell_run_cmd(struct shell_config *sc, char *shell_input);

/* needed for rb_cb_getline() and such callback function */
static struct shell_config *shell_config;

void rl_cb_getline(char* line)
{
    shell_config->shell_input = line;
    if (line == NULL) {
        shell_config->shell_quit = 1;
        return;
    }
}

int is_interactive_mode(struct shell_config *sc)
{
    return !(sc->flags & SF_SCRIPT_MODE);
}

static void handle_signals(int signo) {
    switch (signo) {
        case SIGINT:
            if (rl_line_buffer) {
                rl_replace_line("", 0);
                rl_redisplay();
            }
            break;

        default:
            break;
    }

    if (shell_config->signal_event_hook)
        shell_config->signal_event_hook(signo);
}

void shell_init(struct shell_config *sc)
{
    shell_config = sc;

    sc->shell_input = NULL;
    sc->history_file = NULL;

    if (STAILQ_EMPTY(&sc->inputs_list))
        shell_input_add(sc, SHELL_INPUT_TYPE_STDIO);

    if (!is_interactive_mode(sc)) {
        sc->prompt = strdup("");
    } else {
        if (sc->prompt == NULL)
            sc->prompt = strdup("> ");

        shell_history_init(sc);
        shell_completion_init(sc);

        /* Allow conditional parsing of the ~/.inputrc file. */
        rl_readline_name = sc->progname;
    }

    if (signal(SIGINT, handle_signals) == SIG_ERR) {
        logger(WARN_LOG, "Failed set signal handler\n");
    }
    rl_set_signals();

    shell_input_init_current(sc);
}

void shell_deinit(struct shell_config *sc)
{
    if (is_interactive_mode(sc)) {
        rl_clear_visible_line();
        shell_history_deinit(sc);
    }

    rl_callback_handler_remove();
    rl_clear_signals();
    signal(SIGINT, SIG_IGN);

    free(sc->prompt);
}

int  shell_handle(struct shell_config *sc)
{
    int rc = 0;
    char *cmd;
    if (sc->shell_quit) {
        struct shell_input *si = shell_input_next(sc);

        if (si == NULL)
            return SHELL_EOF;

        /* Hack to show prompt after `source` */
        if (is_interactive_mode(sc)) {
            rl_stuff_char('\n');
            rl_callback_read_char();
        }
    }

    if (!sc->shell_input)
        return 0;

    cmd = strchopspaces(sc->shell_input);
    if (!cmd || cmd[0] == 0 || cmd[0] == '#')
        return 0;

    shell_add_history(shell_config, cmd);

    rc = shell_run_cmd(sc, cmd);

    free(sc->shell_input);
    sc->shell_input = NULL;

    return rc;
}

void shell_make_argv(char *cmd_line, char ***argv, int *argc)
{
    char *p;
    *argc = 0;
    *argv = calloc(1, sizeof(char *));

    for (p = strtok(cmd_line, " "); p; p = strtok(NULL, " ")) {
        (*argv)[(*argc)++] = p;
        *argv = realloc(*argv, (*argc + 1) * sizeof(char *));
    }

    (*argv)[*argc] = NULL;
}

int shell_run_cmd(struct shell_config *sc, char *shell_input)
{
    struct commands_t *cmd;
    char **argv;
    int argc;
    int rc;
    char *cmd_line;

    if (!shell_input || *shell_input == 0)
        return -1;

    cmd_line = strdup(shell_input);

    logger(DEBUG_LOG, "call: %s\n", cmd_line);
    shell_make_argv(cmd_line, &argv, &argc);
    if (argv == NULL || *argv == NULL) {
        free(cmd_line);
        return -1;
    }

    for (cmd = sc->commands; cmd->name != NULL; cmd++) {
        if (!strcmp(cmd->name, argv[0])) {
            if (is_interactive_mode(sc))
                rl_clear_visible_line();

            rc = cmd->handler(sc, argv, argc);

            shell_forced_update_display(sc);
            break;
        }
    }

    if (cmd->name == NULL) {
        fprintf(stderr, "\rUnknown command: %s\n", argv[0]);
        shell_forced_update_display(sc);
    }

    free(cmd_line);
    free(argv);
    return rc;
}

void shell_forced_update_display(struct shell_config *sc)
{
    if (is_interactive_mode(sc))
        rl_forced_update_display();
}

int rl_hook_argv_getch(FILE *in)
{
    int ch;
    struct shell_input *si;
    in = in;

    si = STAILQ_FIRST(&shell_config->inputs_list);

    if (*si->pos == 0)
        return EOF;

    ch = *si->pos++;

    if (ch == 0) {
        ch = '\n';
    } else if (ch == ';')
        ch = '\n';

    return ch;
}

void shell_update_prompt(struct shell_config *sc, char *fmt, ...)
{
    va_list ap;
    char *line_buf = NULL;

    if (rl_line_buffer)
        line_buf = strdup(rl_line_buffer);

    if (sc->prompt)
        free(sc->prompt);

    va_start(ap, fmt);
    vasprintf(&sc->prompt, fmt, ap);
    va_end(ap);

    rl_callback_handler_install(sc->prompt, rl_cb_getline);

    if (line_buf) {
        rl_insert_text(line_buf);
        free(line_buf);
    }

    rl_refresh_line(0, 0);
}

void shell_input_init(struct shell_config *sc)
{
    STAILQ_INIT(&sc->inputs_list);
    sc->inputs_count = 0;
}

int shell_input_add(struct shell_config *sc, int flags, ...)
{
    va_list ap;
    struct shell_input *si;
    int type = flags & SHELL_INPUT_MASK_TYPE;

    if (sc->inputs_count >= SHELL_MAX_INPUT)
        return -2;

    si = malloc(sizeof(struct shell_input));
    if (flags & SHELL_INPUT_PUT_HEAD)
        STAILQ_INSERT_HEAD(&sc->inputs_list, si, next_input);
    else
        STAILQ_INSERT_TAIL(&sc->inputs_list, si, next_input);

    si->flags = flags;

    switch (type) {
        case SHELL_INPUT_TYPE_STDIO:
            si->input = stdin;
            si->source_name = "stdin";
            logger(DEBUG_LOG, "Use stdin.\n");
            break;

        case SHELL_INPUT_TYPE_FILE:
            va_start(ap, flags);
            si->script_file = va_arg(ap, char *);
            va_end(ap);

            if ((si->input = fopen(si->script_file, "r")) == NULL)
                return -1;
            logger(DEBUG_LOG, "Open script file \"%s\".\n", si->script_file);

            break;

        case SHELL_INPUT_TYPE_ARGV:
            va_start(ap, flags);
            si->script_string = va_arg(ap, char *);
            si->pos = si->script_string;
            va_end(ap);

            /* just dummy for select() to make it always ready to read */
            if ((si->input = fopen("/dev/zero", "r")) == NULL)
                return -1;
            logger(DEBUG_LOG, "Add command from command line \"%s\".\n", si->script_string);

            break;

        default:
            assert(!"shell_add_input: unknown input type\n");
    }

    sc->inputs_count++;
    return 0;
}

void shell_input_init_current(struct shell_config *sc)
{
    struct shell_input *si = STAILQ_FIRST(&sc->inputs_list);

    if (!si)
        return;

    switch (si->flags & SHELL_INPUT_MASK_TYPE) {
        case SHELL_INPUT_TYPE_STDIO:
        case SHELL_INPUT_TYPE_FILE:
            if (isatty(fileno(si->input)))
                si->flags |= SHELL_INPUT_INTERACTIVE;
            else
                si->flags &= ~SHELL_INPUT_INTERACTIVE;

            rl_getc_function = rl_getc;
            break;

        case SHELL_INPUT_TYPE_ARGV:
            rl_getc_function = rl_hook_argv_getch;
            break;

        default:
            ;
    }

    if (si->flags & SHELL_INPUT_INTERACTIVE) {
        sc->flags &= ~SF_SCRIPT_MODE;
        rl_outstream = stdout;
    } else {
        static FILE *dev_null = NULL;
        if (!dev_null)
            dev_null = fopen("/dev/null", "w");;
        sc->flags |= SF_SCRIPT_MODE;
        rl_outstream = dev_null;
        rl_tty_set_echoing(0);
    }

    rl_instream = sc->input = si->input;
    sc->shell_quit = 0;
}

struct shell_input* shell_input_next(struct shell_config *sc)
{
    struct shell_input *si = STAILQ_FIRST(&sc->inputs_list);

    STAILQ_REMOVE_HEAD(&sc->inputs_list, next_input);
    free(si);

    if (STAILQ_EMPTY(&sc->inputs_list))
        return NULL;

    si = STAILQ_FIRST(&sc->inputs_list);

    sc->inputs_count--;
    logger (DEBUG_LOG, "Init next source \"%s\"\n", si->source_name);

    shell_input_init_current(sc);

    return si;
}
