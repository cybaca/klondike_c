#ifndef SOLITAIRE_CARD_TYPE_H_
#define SOLITAIRE_CARD_TYPE_H_

#include "list.h"
#include <stdbool.h>

#define NUM_TABLEAU 7
#define NUM_FOUNDATION 4

#define _PILE_IS_TABLEAU(pile) (\
    ((pile)->location >= LOC_TAB0) && ((pile)->location <= LOC_TAB6))

#define _PILE_IS_FOUNDATION(pile) (\
    ((pile)->location >= LOC_FOUND0) && ((pile)->location <= LOC_FOUND3))

enum card_suit {
    SUIT_SPADE,
    SUIT_DIAMOND,
    SUIT_CLUB,
    SUIT_HEART,
    SUIT_MAX,
};


enum card_rank {
    RANK_A, // 0
    RANK_2, // 1
    RANK_3, // 10
    RANK_4, // 11
    RANK_5, // 100
    RANK_6, // 101
    RANK_7, // 110
    RANK_8, // 111
    RANK_9, // 1000
    RANK_10, // 1001
    RANK_J,  // 1010
    RANK_Q, // 1011
    RANK_K, // 1100
    // RANK_JOKER,
    RANK_MAX,
};

enum card_color {
    COLOR_RED,
    COLOR_BLACK,
    COLOR_MAX,
};

enum card_location {
    LOC_DECK,
    LOC_STOCK,
    LOC_WASTE,
    LOC_TAB0,
    LOC_TAB1,
    LOC_TAB2,
    LOC_TAB3,
    LOC_TAB4,
    LOC_TAB5,
    LOC_TAB6,
    LOC_FOUND0,
    LOC_FOUND1,
    LOC_FOUND2,
    LOC_FOUND3,
};


struct pile {
    struct list_head list;
    int len;
    int cap;
    enum card_location location;
};

struct card {
    enum card_suit suit;
    enum card_rank rank;
    enum card_color color;
    bool face_up;
    enum card_location location;
    struct pile *pile;
    struct list_head list;
};

static inline enum card_rank
card_rank(struct card *card)
{
    return card->rank;
}

static inline enum card_suit
card_suit(struct card *card)
{
    return card->suit;
}

static inline bool
card_is_face_up(struct card *card)
{
    return card->face_up;
}

static inline struct pile *
card_pile(struct card *card)
{
    return card->pile;
}

struct deck {
    struct card *cards;
    struct list_head list;
    int len;
    bool initialized;
};


struct card_action {
    struct card *card;
    struct pile *src;
    struct pile *dst;
};

struct history {
    struct card_action *actions;
    int cnt;
    int cap;
};

struct field {
    struct deck *deck;
    struct pile stock;
    struct pile waste;
    struct pile tableaus[NUM_TABLEAU];
    struct pile foundations[NUM_FOUNDATION];
    struct history history;
    int moves;
};

#endif  // SOLITAIRE_CARD_TYPE_H_
