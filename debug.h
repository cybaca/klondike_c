#ifndef SOLITAIRE_DEBUG_H_
#define SOLITAIRE_DEBUG_H_
#include "card_type.h"

void
card_print(struct card *card);

void
card_list_print(struct list_head *head);

void
deck_print(struct deck *deck);

void
pile_print(struct pile *pile);

void
field_print_top_cards(struct field *field);

void
field_sym_print(struct field *field);

void
field_print(struct field *field);

#endif // SOLITAIRE_DEBUG_H_
