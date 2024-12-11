#include "card_type.h"
#include "game.h"
// #include "test.h"
#include "debug.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

static struct termios old_term;

void
set_normal_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

void
set_raw_mode(void)
{
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    atexit(set_normal_mode);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}


// TODO: Add help message for -h flag
int
main(void)
{

    set_raw_mode();
    struct deck deck = { 0 };
    struct field field = { 0 };

    deck_init(&deck);
    field_init(&field, &deck);

    field_sym_print(&field);

    while (!game_over(&field)) {
        if (user_input(&field)) {
            field_sym_print(&field);
        }
        // user_input(&field);
    }

    deck_destroy(&deck);
    field_destroy(&field);
    return 0;
}

