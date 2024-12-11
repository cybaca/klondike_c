#include "test.h"
#include "debug.h"
#include <stdio.h>
#include <assert.h>

typedef bool (*FieldTestFunc)(struct field *field);

#define ARG_STR(str) #str
#define field_test_func(func, arg, ret) do {\
    printf("%s\n", ARG_STR(func)); \
    ret = func(arg); \
} while (0)

#define PFUNC printf("Test: %s\n", __func__)

void
init_game(struct field *field, struct deck *deck)
{
    deck_init(deck);
    field_init(field, deck);
}

void
destroy_game(struct field *field)
{
    deck_destroy(field->deck);
    field_destroy(field);
}

bool
stock_turnover_is_in_order(struct field *field)
{
    PFUNC;

    struct card *card = pile_top_card(&field->stock);
    int i;
    for (i = 0; i < 25; ++i)
        deal_card(field);

    if (card == pile_top_card(&field->stock))
        return true;

    return false;
}

bool
undo_waste_15x_same_top_card(struct field *field)
{
    PFUNC;
    int i;
    struct card *top = pile_top_card(&field->stock);
    for (i = 0; i < 15; ++i)
        deal_card(field);
    for (i = 0; i < 15; ++i)
        undo_move(field);
    if (top == pile_top_card(&field->stock))
        return true;
    return false;
}

bool
undo_waste_30x_same_top_card(struct field *field)
{
    PFUNC;
    int i;
    struct card *top = pile_top_card(&field->stock);
    for (i = 0; i < 30; ++i)
        deal_card(field);
    for (i = 0; i < 30; ++i)
        undo_move(field);
    if (top == pile_top_card(&field->stock))
        return true;
    return false;
}

bool
test3(struct field *field)
{
    return true;
}

/*
static inline bool
_test_move_pile(
    struct pile *src_pile,
    struct pile *dst_pile,
    struct field *field
    )
{
    struct card *src_card = pile_top_card(src_pile);
    if (!move_card_to_pile(src_card, dst_pile)) {
        fprintf(stderr, "move_card_to_pile() failed\n");
        return false;
    }
    pile_top_flip_up(src_pile);
    field_sym_print(field);
    return true;
}
*/


int
run_tests(void)
{
    FieldTestFunc tests[4] = {
        stock_turnover_is_in_order,
        undo_waste_15x_same_top_card,
        undo_waste_30x_same_top_card,
        test3
    };


    struct deck deck = { 0 };
    struct field field = { 0 };

    // deck_init(&deck);
    // field_init(&field, &deck);

    // field_sym_print(&field);

    int i;
    for (i = 0; i < 4; ++i) {
        init_game(&field, &deck);
        if (tests[i](&field) == true)
            printf("Passed test %i\n", i + 1);
        else
            printf("Failed test %i\n", i + 1);
        destroy_game(&field);
    }

    return 1;

}

int
main(void)
{
    run_tests();
    return 0;
}
