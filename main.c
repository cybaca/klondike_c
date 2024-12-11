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

// TODO: Add help message for -h flag
int
main(void)
{
    struct deck deck = { 0 };
    struct field field = { 0 };

    deck_init(&deck);
    field_init(&field, &deck);

    field_sym_print(&field);

    while (!game_over(&field)) {
        if (user_move(&field)) {
            field_sym_print(&field);
        }
    }

    deck_destroy(&deck);
    field_destroy(&field);
    return 0;
}

