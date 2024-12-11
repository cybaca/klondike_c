#ifndef SOLITAIRE_GAME_H_
#define SOLITAIRE_GAME_H_

#include "card_type.h"
#include <stdint.h>


// PILE_FUNCTIONS
/**
 * pile_top_card - Get the top card in a pile.
 * @ pile: struct pile * from which to get the top card
 *
 * Returns the top card from the list_head in the pile struct * or null if
 * empty.
 */
struct card *
pile_top_card(struct pile *pile);

struct card *
pile_last_card(struct pile *pile);

struct card *
pile_get_nth_card(struct pile *pile, int n);

bool
pile_empty(struct pile *pile);

bool
pile_top_is_face_up(struct pile *pile);

struct card *
pile_last_face_up_card(struct pile *pile);

int
pile_count(struct pile *pile);

struct card *
pile_search(struct pile *pile, enum card_suit suit, enum card_rank rank);

bool
pile_has_card(struct pile *pile, struct card *card);

void
pile_top_flip_up(struct pile *pile);

bool
card_is_top_of_pile(struct card *card);

// DECK FUNCTIONS
void
deck_init(struct deck *deck);

void
deck_destroy(struct deck *deck);

struct card *
field_search(
    struct field *field,
    enum card_suit suit,
    enum card_rank rank
    );

// MOVE FUNCTIONS
bool
deal_card(struct field *field);

bool
move_card_to_pile(struct card *src_card, struct pile *dst_pile);

bool
move_card_to_card(struct card *src, struct card *dst);



// FIELD FUNCTIONS
void
field_snapshot_init(struct field *field);

void
field_init(struct field *field, struct deck *deck);

void
field_destroy(struct field *field);

void
field_snapshot(struct field *field);

void
undo_move(struct field *field);


bool
game_over(struct field *field);

bool
user_input(struct field *field);


#endif // SOLITAIRE_GAME_H_
