/* 03-terminal-input.c - Terminal input handling implementation
 *
 * Implements command reading, parsing, and history navigation.
 * Uses ncurses for input handling with support for arrow key navigation.
 */

#include "03-terminal-input.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* {{{ Command strings for parsing */
static const struct {
    char key;
    const char* alt_key;
    CommandType type;
} COMMAND_KEYS[] = {
    { 'p', "play",   CMD_PLAY },
    { 'b', "buy",    CMD_BUY },
    { 'a', "attack", CMD_ATTACK_PLAYER },
    { 's', "scrap",  CMD_SCRAP },
    { 'x', "use",    CMD_ACTIVATE },
    { 'e', "end",    CMD_END_TURN },
    { 'h', "help",   CMD_HELP },
    { '?', NULL,     CMD_HELP },
    { 'q', "quit",   CMD_QUIT },
    { 0, NULL, CMD_NONE }
};
/* }}} */

/* {{{ command_parse
 * Parse a command string into a Command structure.
 * Format: <action> [target] [secondary]
 * Examples: "p 2", "b 0", "a 1", "e", "h"
 */
Command command_parse(const char* input) {
    Command cmd = { CMD_NONE, -1, -1, "" };

    if (input == NULL || input[0] == '\0') {
        return cmd;
    }

    /* Store raw input for error messages */
    strncpy(cmd.raw, input, sizeof(cmd.raw) - 1);
    cmd.raw[sizeof(cmd.raw) - 1] = '\0';

    /* Skip leading whitespace */
    while (*input && isspace(*input)) input++;

    if (*input == '\0') {
        return cmd;
    }

    /* Check for single-character commands */
    char first = tolower(input[0]);

    for (int i = 0; COMMAND_KEYS[i].key != 0; i++) {
        if (first == COMMAND_KEYS[i].key) {
            cmd.type = COMMAND_KEYS[i].type;
            break;
        }
    }

    /* If not found, check for word commands */
    if (cmd.type == CMD_NONE) {
        char word[32];
        if (sscanf(input, "%31s", word) == 1) {
            for (int i = 0; COMMAND_KEYS[i].key != 0; i++) {
                if (COMMAND_KEYS[i].alt_key &&
                    strcasecmp(word, COMMAND_KEYS[i].alt_key) == 0) {
                    cmd.type = COMMAND_KEYS[i].type;
                    break;
                }
            }
        }

        if (cmd.type == CMD_NONE) {
            cmd.type = CMD_UNKNOWN;
            return cmd;
        }
    }

    /* Parse target arguments */
    const char* args = input + 1;
    while (*args && !isspace(*args)) args++;  /* Skip command word */
    while (*args && isspace(*args)) args++;   /* Skip whitespace */

    if (*args) {
        int n = sscanf(args, "%d %d", &cmd.target, &cmd.secondary);

        /* Attack with target becomes ATTACK_BASE */
        if (cmd.type == CMD_ATTACK_PLAYER && n >= 1 && cmd.target >= 0) {
            cmd.type = CMD_ATTACK_BASE;
        }
    }

    return cmd;
}
/* }}} */

/* {{{ command_type_to_string
 * Convert command type to human-readable string.
 */
const char* command_type_to_string(CommandType type) {
    switch (type) {
        case CMD_NONE:          return "none";
        case CMD_PLAY:          return "play";
        case CMD_BUY:           return "buy";
        case CMD_ATTACK_PLAYER: return "attack";
        case CMD_ATTACK_BASE:   return "attack base";
        case CMD_SCRAP:         return "scrap";
        case CMD_ACTIVATE:      return "activate";
        case CMD_END_TURN:      return "end turn";
        case CMD_HELP:          return "help";
        case CMD_QUIT:          return "quit";
        case CMD_UNKNOWN:       return "unknown";
        default:                return "???";
    }
}
/* }}} */

/* {{{ command_is_valid
 * Check if a command is valid (known type, appropriate targets).
 */
bool command_is_valid(Command* cmd) {
    if (cmd == NULL) return false;

    switch (cmd->type) {
        case CMD_NONE:
        case CMD_UNKNOWN:
            return false;

        case CMD_PLAY:
        case CMD_BUY:
        case CMD_SCRAP:
        case CMD_ACTIVATE:
        case CMD_ATTACK_BASE:
            /* These require a target */
            return cmd->target >= 0;

        case CMD_ATTACK_PLAYER:
        case CMD_END_TURN:
        case CMD_HELP:
        case CMD_QUIT:
            /* These don't require targets */
            return true;

        default:
            return false;
    }
}
/* }}} */

/* {{{ terminal_read_command
 * Read a command from the input window with history support.
 * Uses line editing with up/down arrow history navigation.
 */
Command terminal_read_command(TerminalUI* ui, InputHistory* history) {
    static char buffer[INPUT_MAX_LEN];
    int pos = 0;
    int ch;

    if (ui == NULL || ui->input_win == NULL) {
        return command_parse(NULL);
    }

    /* Reset history position for new input */
    if (history) {
        input_history_reset_position(history);
    }

    /* Clear and show prompt */
    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    mvwprintw(ui->input_win, 0, 0, " > ");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    wrefresh(ui->input_win);

    /* Enable cursor and input */
    curs_set(1);
    echo();
    nodelay(ui->input_win, FALSE);  /* Blocking input */

    buffer[0] = '\0';
    pos = 0;

    while (1) {
        ch = wgetch(ui->input_win);

        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        }

        if (ch == KEY_UP && history) {
            const char* prev = input_history_prev(history);
            if (prev) {
                strncpy(buffer, prev, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                pos = strlen(buffer);

                /* Redraw input line */
                werase(ui->input_win);
                wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
                mvwprintw(ui->input_win, 0, 0, " > ");
                wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
                mvwprintw(ui->input_win, 0, 3, "%s", buffer);
                wrefresh(ui->input_win);
            }
            continue;
        }

        if (ch == KEY_DOWN && history) {
            const char* next = input_history_next(history);
            if (next) {
                strncpy(buffer, next, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
            } else {
                buffer[0] = '\0';
            }
            pos = strlen(buffer);

            /* Redraw input line */
            werase(ui->input_win);
            wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 0, " > ");
            wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 3, "%s", buffer);
            wrefresh(ui->input_win);
            continue;
        }

        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && pos > 0) {
            pos--;
            buffer[pos] = '\0';

            /* Redraw input line */
            werase(ui->input_win);
            wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 0, " > ");
            wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 3, "%s", buffer);
            wrefresh(ui->input_win);
            continue;
        }

        if (ch >= 32 && ch < 127 && pos < INPUT_MAX_LEN - 1) {
            buffer[pos++] = ch;
            buffer[pos] = '\0';
        }
    }

    /* Restore cursor state */
    curs_set(0);
    noecho();
    nodelay(ui->input_win, TRUE);

    /* Add to history if non-empty */
    if (buffer[0] != '\0' && history) {
        input_history_add(history, buffer);
    }

    return command_parse(buffer);
}
/* }}} */

/* {{{ terminal_show_prompt
 * Display a prompt in the input window.
 */
void terminal_show_prompt(TerminalUI* ui, const char* prompt) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    mvwprintw(ui->input_win, 0, 0, " %s ", prompt ? prompt : ">");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ input_history_create
 * Create a new input history buffer.
 */
InputHistory* input_history_create(void) {
    InputHistory* history = malloc(sizeof(InputHistory));
    if (history == NULL) return NULL;

    history->count = 0;
    history->position = -1;

    return history;
}
/* }}} */

/* {{{ input_history_free
 * Free an input history buffer.
 */
void input_history_free(InputHistory* history) {
    if (history != NULL) {
        free(history);
    }
}
/* }}} */

/* {{{ input_history_add
 * Add a command to the history buffer.
 */
void input_history_add(InputHistory* history, const char* input) {
    if (history == NULL || input == NULL || input[0] == '\0') return;

    /* Don't add duplicates of the last entry */
    if (history->count > 0 &&
        strcmp(history->entries[history->count - 1], input) == 0) {
        return;
    }

    /* Shift entries if full */
    if (history->count >= INPUT_HISTORY_SIZE) {
        memmove(history->entries[0], history->entries[1],
                (INPUT_HISTORY_SIZE - 1) * INPUT_MAX_LEN);
        history->count = INPUT_HISTORY_SIZE - 1;
    }

    /* Add new entry */
    strncpy(history->entries[history->count], input, INPUT_MAX_LEN - 1);
    history->entries[history->count][INPUT_MAX_LEN - 1] = '\0';
    history->count++;
    history->position = -1;
}
/* }}} */

/* {{{ input_history_prev
 * Navigate to previous history entry.
 */
const char* input_history_prev(InputHistory* history) {
    if (history == NULL || history->count == 0) return NULL;

    if (history->position < 0) {
        /* Start at most recent */
        history->position = history->count - 1;
    } else if (history->position > 0) {
        history->position--;
    } else {
        /* Already at oldest, stay there */
        return history->entries[0];
    }

    return history->entries[history->position];
}
/* }}} */

/* {{{ input_history_next
 * Navigate to next history entry.
 */
const char* input_history_next(InputHistory* history) {
    if (history == NULL || history->position < 0) return NULL;

    if (history->position < history->count - 1) {
        history->position++;
        return history->entries[history->position];
    }

    /* Past end of history, return to new input */
    history->position = -1;
    return NULL;
}
/* }}} */

/* {{{ input_history_reset_position
 * Reset history navigation position.
 */
void input_history_reset_position(InputHistory* history) {
    if (history != NULL) {
        history->position = -1;
    }
}
/* }}} */

/* {{{ terminal_show_help
 * Display command help in the narrative window.
 */
void terminal_show_help(TerminalUI* ui) {
    if (ui == NULL || ui->narrative_win == NULL) return;

    werase(ui->narrative_win);
    box(ui->narrative_win, 0, 0);
    mvwprintw(ui->narrative_win, 0, 2, " COMMANDS ");

    int y = 2;
    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_TRADE));
    mvwprintw(ui->narrative_win, y++, 2, "p <n>  - Play card n from hand");
    mvwprintw(ui->narrative_win, y++, 2, "b <n>  - Buy card n from trade row");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_TRADE));

    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_COMBAT));
    mvwprintw(ui->narrative_win, y++, 2, "a      - Attack opponent");
    mvwprintw(ui->narrative_win, y++, 2, "a <n>  - Attack opponent's base n");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_COMBAT));

    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->narrative_win, y++, 2, "s <n>  - Scrap card n");
    mvwprintw(ui->narrative_win, y++, 2, "x <n>  - Activate base ability n");
    mvwprintw(ui->narrative_win, y++, 2, "e      - End your turn");
    y++;
    mvwprintw(ui->narrative_win, y++, 2, "h or ? - Show this help");
    mvwprintw(ui->narrative_win, y++, 2, "q      - Quit game");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    wrefresh(ui->narrative_win);
}
/* }}} */

/* {{{ terminal_show_error
 * Display an error message in the input window.
 */
void terminal_show_error(TerminalUI* ui, const char* message) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    mvwprintw(ui->input_win, 0, 0, " Error: %s ", message ? message : "Unknown error");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ terminal_show_message
 * Display a message in the input window.
 */
void terminal_show_message(TerminalUI* ui, const char* message) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
    mvwprintw(ui->input_win, 0, 0, " %s ", message ? message : "");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
    wrefresh(ui->input_win);
}
/* }}} */
