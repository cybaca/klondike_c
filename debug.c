#include "debug.h"
#include "game.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define u32_to_u8(u) \
    (((u) >> 16) & 0xff), \
    (((u) >>  8) & 0xff), \
    (((u) >>  0) & 0xff)

#define _print_clear() printf("\x1b[0m")

static inline void
_print_color(uint32_t color);

static inline void
_top_card_print(struct pile *pile);

static inline void
_pile_type_print(struct pile *pile);

static inline void
_card_print(struct card *card, bool with_color);

static inline void
_card_sym_puts(struct card *card);

static inline void
_card_sym_print(struct card *card);

static inline int
_pile_count(struct pile *pile);

bool
_pile_len_matches_count(struct pile *pile);

static inline int
_pile_count(struct pile *pile)
{
    struct card *card;
    int cnt = 0;
    list_for_each_entry(card, &pile->list, list)
        cnt++;
    return cnt;
}

bool
_pile_len_matches_count(struct pile *pile)
{
    int cnt = _pile_count(pile);
    return pile->len == cnt;
}


static inline void
_print_color(uint32_t color)
{
    printf("\x1b[38;2;%u;%u;%um", u32_to_u8(color));
}

static inline void
_top_card_print(struct pile *pile)
{
    if (pile_empty(pile)) {
        _pile_type_print(pile);
        printf(": ");
        printf("Pile empty!\n");
        return;
    }

    struct card *card = pile_top_card(pile);
    if (card == NULL) {
        fprintf(stderr, "Error! Pile not empty but can't retrieve top card!\n");
        return;
    }

    printf("Top card of ");
    _pile_type_print(pile);
    printf(": ");
    _card_sym_puts(card);
}

static inline void
_pile_type_print(struct pile *pile)
{

    char const *typestr[] = {
        "Deck",
        "Stock",
        "Waste",
        "Tableau 1",
        "Tableau 2",
        "Tableau 3",
        "Tableau 4",
        "Tableau 5",
        "Tableau 6",
        "Tableau 7",
        "Foundation 1",
        "Foundation 2",
        "Foundation 3",
        "Foundation 4",
    };

    printf("%s", typestr[pile->location]);
}

static inline void
_card_print(struct card *card, bool with_color)
{
    char const *ranks[] = {
        "Ace", "Two", "Three", "Four", "Five",
        "Six", "Seven", "Eight", "Nine",
        "Ten", "Jack", "Queen", "King", "Joker"
    };

    char const *suits[] = {
        "Spades", "Diamonds", "Clubs", "Hearts"
    };

    char const *colors[] = { "Red", "Black", "No color" };

    char const *flipped[] = { "Face Down", "Face up" };

    uint32_t const clrstr[] = {
        0xd72638, 0xd5d5d5
    };

    if (with_color) {
        printf("%s %s of %s\n",
            colors[card->color],
            ranks[card->rank],
            suits[card->suit]);
        return;
    }
    _print_color(clrstr[card->color]);
    printf("%s of %s - %s\n",
        ranks[card->rank],
        suits[card->suit],
        flipped[card->face_up]
        );
    _print_clear();
}

static inline void
_card_sym_puts(struct card *card)
{
    char const *rankstr[] = {
        "A", "2", "3", "4", "5", "6",
        "7", "8", "9", "X", "J", "Q", "K"
    };

    char const *suitstr[] = {
        "♠", "♦", "♣", "♥",
        // "♤", "♢", "♧", "♡"
    };

    uint32_t const clrstr[] = {
        0xd72638, 0xd5d5d5
    };

    _print_color(clrstr[card->color]);
    printf("%s%s", rankstr[card->rank], suitstr[card->suit]);
    _print_clear();
    printf("\n");
}

static inline void
_card_sym_print(struct card *card)
{
    assert(card != NULL);
    char const *rankstr[] = {
        "A", "2", "3", "4", "5", "6",
        "7", "8", "9", "X", "J", "Q", "K"
    };

    char const *suitstr[] = {
        "♠", "♦", "♣", "♥",
        // "♤", "♢", "♧", "♡"
    };

    uint32_t const clrstr[] = {
        0xd72638, 0xd5d5d5
    };

    _print_color(clrstr[card->color]);
    printf("%s%s", rankstr[card->rank], suitstr[card->suit]);
    _print_clear();
}

void
card_print(struct card *card)
{
    assert(card != NULL);
    _card_sym_print(card);
    printf("\n");
}


void
card_list_print(struct list_head *head)
{
    if (list_empty(head)) {
        printf("List empty!\n");
        return;
    }

    struct card *card;

    list_for_each_entry(card, head, list)
        _card_print(card, false);
}

void
deck_print(struct deck *deck)
{
    int i;
    for (i = 0; i < deck->len; ++i)
        _card_print(deck->cards + i, true);
}

void
pile_print(struct pile *pile)
{
    if (list_empty(&pile->list)) {
        printf("List empty!\n");
        return;
    }

    struct card *card;
    list_for_each_entry(card, &pile->list, list)
        _card_sym_puts(card);
}


void
field_print_top_cards(struct field *field)
{
    _top_card_print(&field->stock);
    _top_card_print(&field->waste);
    int i;
    for (i = 0; i < NUM_TABLEAU; ++i)
        _top_card_print(&field->tableaus[i]);
    for (i = 0; i < NUM_FOUNDATION; ++i)
        _top_card_print(&field->foundations[i]);
    printf("\n");
}

// TODO: Fix tableaus so that they are aligned with the top of the field
// TODO: Write a print functions that shows ALL cards, with unflipped cards
// grayed out
void
field_sym_print(struct field *field)
{
    struct card *card;
    printf("stock/waste: ");
    card = pile_top_card(&field->stock);
    if (card == NULL)
        printf("EE");
    else
        printf("XX");
    // _card_sym_print(card);

    printf("    ");
    card = pile_top_card(&field->waste);
    if (card != NULL)
        _card_sym_print(card);
    else
        printf("  ");
    printf("    foundations: ");

    int i;
    for (i = 0; i < NUM_FOUNDATION; ++i) {
        card = pile_top_card(&field->foundations[i]);
        if (card == NULL)
            printf("XX");
        else
            _card_sym_print(card);
        printf("  ");
    }

    printf("\n");

    int height = 0;
    int counts[NUM_TABLEAU];
    for (i = 0; i < NUM_TABLEAU; ++i) {
        counts[i] = _pile_count(&field->tableaus[i]);
        if (counts[i] > height)
            height = counts[i];
    }

    printf("tableaus:\n");
    int row;
    int col;
    for (row = height - 1; row >= 0; --row) {
        for (col = 0; col < NUM_TABLEAU; ++col) {
            if (row <= counts[col]) {
                card = pile_get_nth_card(&field->tableaus[col], row);
                if (card == NULL)
                    printf("  ");
                else if (card->face_up)
                    _card_sym_print(card);
                else
                    printf("XX");
                printf("  ");
            } else {
                printf("    ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void
field_print(struct field *field)
{
    struct card *card;
    int num = 0;
    int cnt = 0;
    printf("Stock:\n");
    cnt += field->stock.len;
    cnt += field->waste.len;
    list_for_each_entry(card, &field->stock.list, list) {
        // _card_print(card, false);
        _card_sym_puts(card);
        num++;
    }
    assert(_pile_len_matches_count(&field->stock));
    printf("\n");
    printf("Waste:\n");
    list_for_each_entry(card, &field->waste.list, list) {
        // _card_print(card, false);
        _card_sym_puts(card);
        num++;
    }
    assert(_pile_len_matches_count(&field->waste));
    printf("\n");
    int i;
    for (i = 0; i < NUM_TABLEAU; ++i) {
        printf("Tableau %i\n", i + 1);
        if (!list_empty(&field->tableaus[i].list))
            list_for_each_entry(card, &field->tableaus[i].list, list) {
                // _card_print(card, false);
                _card_sym_puts(card);
                num++;
            }
        assert(_pile_len_matches_count(&field->tableaus[i]));
        cnt += field->tableaus[i].len;
        printf("\n");
    }

    for (i = 0; i < NUM_FOUNDATION; ++i) {
        printf("Foundation %i\n", i + 1);
        if (!list_empty(&field->foundations[i].list))
            list_for_each_entry(card, &field->foundations[i].list, list) {
                // _card_print(card, false);
                _card_sym_puts(card);
                num++;
            }
        assert(_pile_len_matches_count(&field->foundations[i]));
        cnt += field->foundations[i].len;
        printf("\n");
    }

    printf("Number of cards counted: %i\n", num);
    printf("Number of cards in counts: %i\n", cnt);
}

