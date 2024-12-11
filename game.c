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

#define SOLITAIRE_DECK_SIZE (RANK_MAX * SUIT_MAX)

#define list_last_entry_or_null(ptr, type, member) ({ \
    struct list_head *head__ = (ptr); \
    struct list_head *pos__ = head__->prev; \
    pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
    })

#define ERR_MSG(msg) assert(msg)
#if !defined ERR_MSG
#define ERR_MSG(msg) \
    fprintf(stderr, "In func %s: ", __func__), \
    fprintf(stderr, msg)
#endif

/**
 * INTERNAL / PRIVATE FUNCTIONS
 */
// UTIL
static inline void
_str_to_lower(char *str);

static inline bool
_char_is_number(char c);

static inline int
_char_to_int(char c);

static inline char
_str_last_char(char *str);

static inline int
_str_get_digit(char *str);

// PILE
static inline void
_pile_set_card_locations(struct pile *pile);

static inline void
_pile_set_card_piles(struct pile *pile);

// DECK
static inline void
_deck_enqueue(struct deck *deck);

static inline void
_deck_shuffle(struct deck *deck);

static inline int
_deck_generate_standard(struct deck *deck);

static inline void
_pile_top_set_location(struct pile *pile);

static inline void
_pile_top_set_pile(struct pile *pile);

static inline bool
_pile_is_waste(struct pile *pile);

static inline bool
_pile_is_tableau(struct pile *pile);

static inline bool
_pile_is_foundation(struct pile *pile);

static inline bool
_pile_is_stock(struct pile *pile);

static inline bool
_card_color_valid(struct card *card);

static inline bool
_tableau_move_valid(struct card *src, struct card *dst);

static inline bool
_foundation_move_valid(struct card *src, struct card *dst);

static inline bool
_move_valid(struct card *src_card, struct pile *dst_pile);

static inline bool
_move_stock_to_waste(struct pile *stock, struct pile *waste);

static inline bool
_move_pile_to_pile_single(struct pile *src_pile, struct pile *dst_pile);

static inline void
_move_stack(struct card *src_card, struct pile *dst_pile);

static inline void
_move_card_to_pile(struct card *src_card, struct pile *dst_pile);

static inline void
_move_card(struct card *src, struct card *dst);

static inline void
_field_history_init(struct field *field);

static inline void
_field_snapshot(struct field *field, struct card_action *act);

static inline struct card_action
_action_store(struct card *card, struct pile *src, struct pile *dst);

static inline struct card_action
_history_pop(struct field *field);

static inline void
_history_push(
    struct field *field,
    struct card *card,
    struct pile *src,
    struct pile *dst
    );

// INTERNAL IMPLEMENTATION

static inline void
_pile_set_card_locations(struct pile *pile)
{
    struct card *card;
    list_for_each_entry(card, &pile->list, list)
        card->location = pile->location;
}

static inline void
_pile_set_card_piles(struct pile *pile)
{
    struct card *card;
    list_for_each_entry(card, &pile->list, list)
        card->pile = pile;
}

static inline void
_pile_top_set_location(struct pile *pile)
{
    struct card *card = pile_top_card(pile);
    if (card == NULL) {
        fprintf(stderr, "CARD WAS NULL UNABLE TO SET LOCATION %s\n", __func__);
        return;
    }
    card->location = pile->location;
}

static inline void
_pile_top_set_pile(struct pile *pile)
{
    struct card *card = pile_top_card(pile);
    if (card == NULL)
        return;
    card->pile = pile;
}

static inline bool
_pile_is_waste(struct pile *pile)
{
    return pile->location == LOC_WASTE;
}

static inline bool
_pile_is_tableau(struct pile *pile)
{
    return pile->location >= LOC_TAB0 && pile->location <= LOC_TAB6;
}

static inline bool
_pile_is_foundation(struct pile *pile)
{
    return pile->location >= LOC_FOUND0 && pile->location <= LOC_FOUND3;
}

static inline bool
_pile_is_stock(struct pile *pile)
{
    return pile->location == LOC_STOCK;
}

static inline bool
_card_color_valid(struct card *card)
{
    if (card == NULL)
        return false;
    return card->color == COLOR_RED || card->color == COLOR_BLACK;
}

static inline bool
_tableau_move_valid(struct card *src_card, struct card *dst_card)
{
    // Assert colors are not out of range
    if (dst_card != NULL)
        assert(_card_color_valid(dst_card));

    // Ensure from card exists
    if (src_card == NULL) {
        ERR_MSG("Source card does not exist\n");
        return false;
    }
    assert(_card_color_valid(src_card));

    // CASE: Tableau is empty so selected card must be king
    if (dst_card == NULL && src_card->rank == RANK_K) {
        return true;
    } else if (dst_card == NULL && src_card->rank != RANK_K) {
        ERR_MSG("No card in dest tableau, but source card is not King!\n");
        return false;
    }

    // CASE: Colors must be opposite
    if (dst_card->color == src_card->color) {
        ERR_MSG("Colors are not opposite!\n");
        return false;
    }

    // CASE: Number must be 1 less to move
    if (src_card->rank != dst_card->rank - 1) {
        ERR_MSG("Dest must be one less than source card\n");
        return false;
    }

    return true;
}

static inline bool
_foundation_move_valid(struct card *src_card, struct card *dst_card)
{
    if (dst_card != NULL) {
        assert(_card_color_valid(dst_card));
        if (!_pile_is_stock(dst_card->pile) && !_pile_is_waste(dst_card->pile))
            assert(dst_card->face_up);
    }

    assert(src_card != NULL);
    assert(_card_color_valid(src_card));
    if (!_pile_is_stock(src_card->pile) && !_pile_is_waste(src_card->pile))
        assert(src_card->face_up);


    // CASE: Foundation is empty
    if (dst_card == NULL) {
        if (src_card->rank == RANK_A)
            return true;
        return false;
    }

    if (dst_card->suit != src_card->suit)
        return false;

    if (dst_card->rank != src_card->rank - 1)
        return false;

    return true;
}

static inline bool
_move_valid(struct card *src_card, struct pile *dst_pile)
{
    assert(src_card != NULL);
    if (_pile_is_foundation(dst_pile))
        return _foundation_move_valid(src_card, pile_top_card(dst_pile));
    if (_pile_is_tableau(dst_pile))
        return _tableau_move_valid(src_card, pile_top_card(dst_pile));
    return false;
}

// DECK
static inline void
_deck_enqueue(struct deck *deck)
{
    struct list_head *head = &deck->list;
    INIT_LIST_HEAD(head);
    int i;
    for (i = 0; i < deck->len; ++i) {
        list_add_tail(&deck->cards[i].list, head);
    }
}

static inline void
_deck_shuffle(struct deck *deck)
{
    srand(time(NULL));
    int i;
    for (i = 0; i < deck->len; ++i) {
        int j = rand() / (RAND_MAX / (deck->len - i) + 1);
        struct card t = deck->cards[j];
        deck->cards[j] = deck->cards[i];
        deck->cards[i] = t;
    }
}

static inline int
_deck_generate_standard(struct deck *deck)
{
    enum card_suit suit;
    enum card_rank rank;

    deck->initialized = true;
    deck->len = SOLITAIRE_DECK_SIZE;
    deck->cards = malloc(sizeof(struct card) * (size_t)deck->len);
    struct card *card = deck->cards;

    for (suit = SUIT_SPADE; suit < SUIT_MAX; ++suit) {
        for (rank = RANK_A; rank < RANK_MAX; ++rank) {
            card->suit = suit;
            card->rank = rank;
            card->color = !(card->suit & 0x1);
            card->face_up = false;
            card->location = LOC_DECK;
            card++;
        }
    }

    return true;
}

static inline bool
_move_all_cards(struct pile *src_pile, struct pile *dst_pile)
{
    if (list_empty(&src_pile->list))
        return false;

    int count = pile_count(src_pile);
    int i;
    for (i = 0; i < count; ++i) {
        struct card *card = pile_top_card(src_pile);
        card->face_up = !card->face_up;
        list_move(src_pile->list.next, &dst_pile->list);
    }
    _pile_set_card_locations(dst_pile);
    _pile_set_card_piles(dst_pile);
    dst_pile->len = src_pile->len;
    src_pile->len = 0;
    return true;
}

static inline bool
_move_stock_to_waste(struct pile *stock, struct pile *waste)
{
    assert(_pile_is_stock(stock));
    assert(_pile_is_waste(waste));

    // CASE: Stock is empty. Move all cards back from waste to stock.
    if (list_empty(&stock->list)) {
        // CASE: Waste is empty too. Can't do anything.
        if (list_empty(&waste->list))
            return false;

        _move_all_cards(waste, stock);
        /*
        int count = pile_count(waste);
        int i;
        for (i = 0; i < count; ++i) {
            struct card *card = pile_top_card(waste);
            card->face_up = false;
            list_move(waste->list.next, &stock->list);
        }
        _pile_set_card_locations(stock);
        _pile_set_card_piles(stock);
        stock->len = waste->len;
        waste->len = 0;
        */

        // This method moves the cards in revered order.
        // TODO: Consider moving the cards to the stock as a queue, and to the
        // pile as a stack. When the pile is empty, list_plice_init can be used
        // to moved the whole pile in fewer operations. However, the cards will
        // still need to be iterated through in order to set locations and piles
        /*
        list_splice_init(&waste->list, &stock->list);
        struct card *card;
        list_for_each_entry(card, &stock->list, list) {
            card->face_up = false;
            card->location = LOC_STOCK;
            card->pile = stock;
        }
        stock->len = waste->len;
        waste->len = 0;
        */
        return true;
    }

    list_move(stock->list.next, &waste->list);
    pile_top_flip_up(waste);
    _pile_top_set_location(waste);
    _pile_top_set_pile(waste);
    waste->len++;
    stock->len--;
    assert(stock->len >= 0);
    return true;
}

static inline void
_move_stack(struct card *src_card, struct pile *dst_pile)
{
    assert(src_card != NULL);
    if (!src_card->face_up) {
        fprintf(stderr, "Card is not face up %s\n", __func__);
        return;
    }

    struct pile *src_pile = src_card->pile;
    if (!_pile_is_tableau(src_pile)) {
        fprintf(stderr, "Source pile is not a tableau %s\n", __func__);
        return;
    }

    struct list_head temp = { 0 };
    INIT_LIST_HEAD(&temp);
    list_cut_position(&temp, &src_pile->list, &src_card->list);
    list_splice_init(&temp, &dst_pile->list);
    _pile_set_card_locations(dst_pile);
    _pile_set_card_piles(dst_pile);
}

static inline void
_move_card_to_pile(struct card *src_card, struct pile *dst_pile)
{
    assert(src_card != NULL);
    /*
    if (!src_card->face_up) {
        fprintf(stderr, "Card is not face up %s\n", __func__);
        return;
    }
    */
    list_move(&src_card->list, &dst_pile->list);
    src_card->pile = dst_pile;
    src_card->location = dst_pile->location;
    src_card->pile->len--;
    dst_pile->len++;
}

static inline void
_move_card(struct card *src, struct card *dst)
{
    if (!(src->face_up && dst->face_up)) {
        fprintf(stderr, "Card is not face up %s\n", __func__);
        return;
    }
    list_move(&src->list, &dst->pile->list);
    src->pile = dst->pile;
    src->location = dst->location;
    src->pile->len--;
    dst->pile->len++;
}

/**
 * EXTERNAL / PUBLIC FUNCTIONS
 */
void
die(char *msg)
{
    perror(msg);
    exit(1);
}



// PILE
struct card *
pile_top_card(struct pile *pile)
{
    return list_first_entry_or_null(&pile->list, struct card, list);
}

struct card *
pile_last_card(struct pile *pile)
{
    return list_last_entry_or_null(&pile->list, struct card, list);
}

struct card *
pile_get_nth_card(struct pile *pile, int n)
{
    if (pile_empty(pile))
        return NULL;

    struct card *card;
    int i = 0;
    list_for_each_entry(card, &pile->list, list) {
        if (i == n)
            return card;
        i++;
    }
    return NULL;
}

bool
pile_empty(struct pile *pile)
{
    return list_empty(&pile->list);
}

bool
pile_top_is_face_up(struct pile *pile)
{
    struct card *card = pile_top_card(pile);
    if (card == NULL)
        return false;
    return card->face_up;
}

int
pile_count(struct pile *pile)
{
    if (pile_empty(pile))
        return 0;
    int i = 0;
    struct card *card;
    list_for_each_entry(card, &pile->list, list)
        i++;
    return i;
}

/*
struct pile *
_get_card_pile(struct card *card, struct field *field)
{
    if (card->location >= LOC_FOUND0 && card->location <= LOC_FOUND3)
        return &field->foundations[card->location - LOC_FOUND0];
    if (card->location >= LOC_TAB0 && card->location <= LOC_TAB6)
        return &field->tableaus[card->location - LOC_TAB0];
    if (card->location == LOC_WASTE)
        return &field->waste;
    if (card->location == LOC_STOCK)
        return &field->stock;
    return NULL;
}

int
_get_card_pile_index(struct card *card)
{
    if (card->location >= LOC_FOUND0 && card->location <= LOC_FOUND3)
        return card->location - LOC_FOUND0;

    if (card->location >= LOC_TAB0 && card->location <= LOC_TAB6)
        return card->location - LOC_TAB0;
    return -1;
}
*/

struct card *
pile_search(struct pile *pile, enum card_suit suit, enum card_rank rank)
{
    struct card *card;
    list_for_each_entry(card, &pile->list, list)
        if (card->suit == suit && card->rank == rank)
            return card;
    return NULL;
}

bool
pile_has_card(struct pile *pile, struct card *card)
{
    struct card *cptr;
    if (list_empty(&pile->list))
        return false;

    if (card == NULL)
        return false;

    list_for_each_entry(cptr, &pile->list, list)
        if (card == cptr)
            return true;
    return false;
}

void
pile_top_flip_up(struct pile *pile)
{
    struct card *card = pile_top_card(pile);
    if (card == NULL)
        return;
    card->face_up = true;
    /*
    if (card->face_up == false)
        card->face_up = true;
    */
}

void
card_flip(struct card *card)
{
    card->face_up = !card->face_up;
}

struct card *
card_next(struct card *card)
{
    struct list_head *head = card->list.next;
    struct list_head *pos = head->next;
    if (pos != head)
        return list_entry(card->list.next, struct card, list);
    return NULL;
}

struct card *
card_prev(struct card *card)
{
    struct list_head *head = card->list.prev;
    struct list_head *pos = head->prev;
    if (pos != head)
        return list_entry(card->list.prev, struct card, list);
    return NULL;
}

bool
card_is_top_of_pile(struct card *card)
{
    return card == pile_top_card(card->pile);
}

void
deck_init(struct deck *deck)
{
    _deck_generate_standard(deck);
    _deck_shuffle(deck);
    _deck_enqueue(deck);
}

void
deck_destroy(struct deck *deck)
{
    if (deck->cards != NULL) {
        free(deck->cards);
        deck->cards = NULL;
        deck->len = 0;
        deck->initialized = false;
    }
}

struct card *
field_search(
    struct field *field,
    enum card_suit suit,
    enum card_rank rank
    )
{
    struct card *card = NULL;
    if ((card = pile_search(&field->stock, suit, rank)) != NULL) {
        return card;
    }

    if ((card = pile_search(&field->waste, suit, rank)) != NULL) {
        return card;
    }

    int i;
    for (i = 0; i < NUM_FOUNDATION; ++i) {
        if ((card =
                pile_search(&field->foundations[i], suit, rank)) != NULL) {
            return card;
        }
    }

    for (i = 0; i < NUM_TABLEAU; ++i) {
        if ((card = pile_search(&field->tableaus[i], suit, rank)) != NULL) {
            return card;
        }
    }

    return NULL;
}

// MOVE FUNCTIONS
static inline bool
_move_pile_to_pile_single(struct pile *src_pile, struct pile *dst_pile)
{
    assert(!pile_empty(src_pile));
    assert(pile_top_is_face_up(src_pile));
    if (!pile_empty(dst_pile))
        assert(pile_top_is_face_up(dst_pile));
    struct card *src_card = pile_top_card(src_pile);
    if (!_move_valid(src_card, dst_pile))
        return false;

    list_move(src_pile->list.next, &dst_pile->list);
    _pile_top_set_location(dst_pile);
    _pile_top_set_pile(dst_pile);
    src_pile->len--;
    dst_pile->len++;
    return true;
}

bool
move_card_to_pile(struct card *src_card, struct pile *dst_pile)
{
    assert(src_card != NULL);
    if (!src_card->face_up) {
        fprintf(stderr, "src_card is not face up! %s\n", __func__);
        return false;
    }
    if (dst_pile->location < LOC_TAB0) {
        fprintf(stderr,
            "dst pile is not a foundation or tableau: %d! %s\n",
            dst_pile->location,
            __func__
            );
        return false;
    }
    struct card *dst_card = pile_top_card(dst_pile);

    // Destination pile is not empty, so move the cards using move_card_to_card
    if (dst_card != NULL)
        return move_card_to_card(src_card, dst_card);

    struct pile *src_pile = src_card->pile;
    if (src_pile->location < LOC_WASTE) {
        fprintf(stderr, "src_pile is < LOC_WASTE! %s\n", __func__);
        return false;
    }

    // If the moving card is on top, then the card can be moved with the
    // _move_pile_to_pile_single functions.
    if (card_is_top_of_pile(src_card))
        return _move_pile_to_pile_single(src_card->pile, dst_pile);

    // If both piles are tableau, can move stack
    if (_pile_is_tableau(src_pile) && _pile_is_tableau(dst_pile)) {
        _move_stack(src_card, dst_pile);
        return true;
    }

    fprintf(stderr,
        "Reached end of function %s without moving card!\n", __func__);

    return false;
}

bool
move_card_to_card(struct card *src_card, struct card *dst_card)
{
    assert(src_card != NULL);
    assert(dst_card != NULL);

    if (!src_card->face_up) {
        fprintf(stderr, "src card is not face up! %s\n", __func__);
        return false;
    }

    if (!dst_card->face_up) {
        fprintf(stderr, "dst card is not face up! %s\n", __func__);
        return false;
    }

    if (src_card->location < LOC_WASTE) {
        fprintf(stderr,
            "src location is not < LOC_WASTE! %s\n", __func__);
        return false;
    }

    if (src_card == dst_card) {
        fprintf(stderr,
            "src card is dst card!%s\n", __func__);
        return false;
    }

    // move card to foundation
    if (_pile_is_foundation(dst_card->pile)) {
        if (!_foundation_move_valid(src_card, dst_card)) {
            fprintf(stderr, "!_foundation_move_valid! %s\n", __func__);
            return false;
        }
        _move_card(src_card, dst_card);
        return true;
    }

    // move card to tableau
    if (_pile_is_tableau(dst_card->pile)) {
        if (!_tableau_move_valid(src_card, dst_card)) {
            fprintf(stderr, "!_tableau_move_valid! %s\n", __func__);
            return false;
        }
        // If card is top of pile, move to tableau regardless of src location
        if (card_is_top_of_pile(src_card)) {
            _move_card(src_card, dst_card);
            return true;
        }

        // If the card is from a tableau, and has not already been moved by the
        // previous if statement, then it is not the top card, and the stack
        // must be moved
        if (_pile_is_tableau(src_card->pile)) {
            _move_stack(src_card, dst_card->pile);
            return true;
        }
    }

    fprintf(stderr, "Reached end of %s without moving card!\n", __func__);
    return false;
}

bool
deal_card(struct field *field)
{
    struct card *card = pile_top_card(&field->stock);
    if (!_move_stock_to_waste(&field->stock, &field->waste)) {
        return false;
    }
    _history_push(field, card, &field->stock, &field->waste);
    // return _move_stock_to_waste(&field->stock, &field->waste);
    return true;
}


// FIELD FUNCTIONS

static inline void
_field_history_init(struct field *field)
{
    struct history *hist = &field->history;
    hist->cnt = 0;
    hist->cap = 32;
    hist->actions = malloc(sizeof(struct card_action) * (size_t)hist->cap);
    if (hist->actions == NULL)
        die("malloc");
}

static inline void
_field_history_destroy(struct field *field)
{
    struct history *hist = &field->history;
    if (hist->actions == NULL)
        return;
    free(hist->actions);
    memset(hist, 0, sizeof(struct history));
}

static inline void
_field_snapshot(struct field *field, struct card_action *act)
{
    struct history *hist = &field->history;


    if (hist->cnt >= hist->cap) {
        hist->cap *= 2;
        hist->actions = realloc(
            hist->actions,
            sizeof(struct card_action) * (size_t)hist->cap
            );
    }

    hist->actions[hist->cnt++] = *act;
}

static inline struct card_action
_action_store(struct card *card, struct pile *src, struct pile *dst)
{
    struct card_action act = { card, src, dst };
    return act;
}

static inline struct card_action
_history_pop(struct field *field)
{
    // struct history *hist = &field->history;
    if (field->history.cnt < 1)
        return _action_store(NULL, NULL, NULL);
    return field->history.actions[--field->history.cnt];
}

static inline void
_history_push(
    struct field *field,
    struct card *card,
    struct pile *src,
    struct pile *dst
    )
{
    struct card_action act = _action_store(card, src, dst);
    _field_snapshot(field, &act);
}


void
undo_move(struct field *field)
{
    struct card_action act = _history_pop(field);
    if (act.card == NULL && act.src == NULL) {
        fprintf(stderr, "No history to undo %s\n", __func__);
        return;
    }

    if (act.card == NULL && act.src->location == LOC_STOCK) {
        _move_all_cards(&field->stock, &field->waste);
        return;
    }

    act.src->len++;
    act.dst->len--;
    _move_card_to_pile(act.card, act.src);
    struct card *card = card_next(act.card);
    if (card != NULL)
        card_flip(card);
}

void
field_init(struct field *field, struct deck *deck)
{
    memset(field, 0, sizeof(struct field));
    field->deck = deck;

    _field_history_init(field);

    // Initialize stock list head
    INIT_LIST_HEAD(&field->stock.list);
    field->stock.location = LOC_STOCK;


    // Initialize waste list head
    INIT_LIST_HEAD(&field->waste.list);
    field->waste.location = LOC_WASTE;

    int i;
    // Initialize all of the tableau list heads
    for (i = 0; i < 7; ++i) {
        INIT_LIST_HEAD(&field->tableaus[i].list);
        field->tableaus[i].location = LOC_TAB0 + i;
    }
    // Initialize all of the foundation list heads
    for (i = 0; i < 4; ++i) {
        INIT_LIST_HEAD(&field->foundations[i].list);
        field->foundations[i].location = LOC_FOUND0 + i;
    }

    // Move all cards from the deck to the stock
    list_splice_tail_init(&deck->list, &field->stock.list);
    field->stock.len = deck->len;
    deck->len = 0;
    _pile_set_card_locations(&field->stock);
    _pile_set_card_piles(&field->stock);

    // Initialize tableaus
    int j;

    for (i = NUM_TABLEAU - 1; i >= 0; --i) {
        for (j = i; j >= 0; --j) {
            // Move the top entry from the stock to each tableau
            list_move(field->stock.list.next, &field->tableaus[i].list);
                field->tableaus[j].cap = i;
                field->tableaus[i].len++;
                field->stock.len--;
        }
    }

    for (i = 0; i < NUM_TABLEAU; ++i) {
        _pile_set_card_locations(&field->tableaus[i]);
        _pile_set_card_piles(&field->tableaus[i]);
    }

    for (i = 0; i < NUM_TABLEAU; ++i) {
        // card = pile_top_card(&field->tableaus[i]);
        //  card->face_up = true;
        pile_top_flip_up(&field->tableaus[i]);
    }

    // Move the top entry from stock to waste
    // list_move(field->stock.list.next, &field->waste.list);
    deal_card(field);
}

void
field_destroy(struct field *field)
{
    _field_history_destroy(field);
}

static inline void
_str_to_lower(char *str)
{
    char *c = str;
    for (c = str; *c != '\0'; ++c)
        *c = tolower(*c);
}

static inline bool
_char_is_number(char c) { return !(c < '0' || c > '9'); }

static inline int
_char_to_int(char c)
{
    if (!_char_is_number(c))
        return -1;
    return c - '0';
}

static inline char
_str_last_char(char *str)
{
    char *c = str;
    while (*c != '\0')
        c++;
    return *(c - 1);
}

static inline int
_str_get_digit(char *str)
{
    char c = _str_last_char(str);
    return _char_to_int(c);
}

// TODO: Write parser for easy commandline input
// TODO: Implement auto move / auto completion
bool
user_move(struct field *field)
{
    char const *rankstr[] = {
        "a", "2", "3", "4", "5", "6",
        "7", "8", "9", "x", "j", "q", "k"
    };

    char const *suitstr[] = {
        "s", "d", "c", "h"
    };

    char move_src[256] = { 0 };
    char move_dst[256] = { 0 };

    printf("Enter next move: \n");
    scanf("%s %s", move_src, move_dst);
    _str_to_lower(move_src);
    _str_to_lower(move_dst);

    printf("Input: %s %s\n", move_src, move_dst);

    if (move_src[0] == 'd') {
        deal_card(field);
        printf("Deal card!\n");
        return true;
    }

    if (move_src[0] == 'u') {
        undo_move(field);
        printf("Undo!\n");
        return true;
    }


    enum card_rank rank = RANK_MAX;
    enum card_suit suit = SUIT_MAX;
    struct card *src_card = NULL;
    struct pile *dst_pile = NULL;

    int i;
    char n = _str_last_char(move_src);
    for (i = 0; i < RANK_MAX; ++i)
        if (move_src[0] == rankstr[i][0])
            rank = i;
    for (i = 0; i < SUIT_MAX; ++i)
        if (n == suitstr[i][0])
            suit = i;

    if (rank == RANK_MAX) {
        fprintf(stderr, "Not a valid number input: %s %d\n", move_src, rank);
        return false;
    }
    if (suit == SUIT_MAX) {
        fprintf(stderr, "Not a valid suit input %s %d\n", move_src, suit);
        return false;
    }

    src_card = field_search(field, suit, rank);
    if (src_card == NULL) {
        fprintf(stderr, "ERROR: Could not find card!!!\n");
        return false;
    }

    // TODO: Need to add logic so that card can not be moved from any position
    // in waste pile.
    if (src_card->face_up == false) {
        printf("That card is not available! Choose another card\n");
        return false;
    }


    n = _str_last_char(move_dst);
    if (move_dst[0] == 't') {
        switch (n) {
            case '1':
                dst_pile = &field->tableaus[0];
                break;
            case '2':
                dst_pile = &field->tableaus[1];
                break;
            case '3':
                dst_pile = &field->tableaus[2];
                break;
            case '4':
                dst_pile = &field->tableaus[3];
                break;
            case '5':
                dst_pile = &field->tableaus[4];
                break;
            case '6':
                dst_pile = &field->tableaus[5];
                break;
            case '7':
                dst_pile = &field->tableaus[6];
                break;
            default:
                fprintf(stderr, "Destination input invalid!\n");
                return false;
        }
    }

    if (move_dst[0] == 'f') {
        switch (n) {
            case '1':
                dst_pile = &field->foundations[0];
                break;
            case '2':
                dst_pile = &field->foundations[1];
                break;
            case '3':
                dst_pile = &field->foundations[2];
                break;
            case '4':
                dst_pile = &field->foundations[3];
                break;
            default:
                fprintf(stderr, "Destination input invalid!\n");
                return false;
        }
    }

    if (dst_pile == NULL) {
        fprintf(stderr, "Invalid destionation  input: %s\n", move_dst);
        return false;
    }

    printf("Moved from %s to %s\n", move_src, move_dst);

    struct pile *flip_pile = src_card->pile;
    bool ret = move_card_to_pile(src_card, dst_pile);
    if (ret)
        _history_push(field, src_card, flip_pile, dst_pile);
    pile_top_flip_up(flip_pile);
    return ret;
}

bool
game_completion_check(struct field *field)
{
    int i;
    for (i = 0; i < NUM_FOUNDATION; ++i) {
        struct card *card = pile_top_card(&field->foundations[i]);
        if (card == NULL)
            return false;
        if (card->rank != RANK_K)
            return false;
    }

    return true;
}

struct card *
pile_last_face_up_card(struct pile *pile)
{
    struct card *card;
    if (pile_empty(pile))
        return NULL;
    list_for_each_entry_reverse(card, &pile->list, list)
        if (card->face_up)
            return card;
    return NULL;
}

bool
dead_end_check(struct field *field)
{
    // 1. Check all tableaus for cards with unflipped neighbords that can be
    // moved to other tableaus or foundations
    // 2. Check all stock/waste cards for possible moves to tableaus and
    // foundations to enable 1.
    // 3. Check if foundations can be moved to tableaus to enable 1.


    int i;
    int j;

    /*
    struct card *kings[SUIT_MAX];
    for (i = 0; i < SUIT_MAX; ++i) {
        kings[i] = field_search(field, SUIT_SPADE + i, RANK_K);
        if (kings[i] == NULL)
            fprintf(stderr, "ERROR: Could not find one or more kings %s\n",
                __func__);
    }
    */

    struct card *src_card;
    struct card *dst_card;
    for (i = 0; i < NUM_TABLEAU; ++i) {
        // If target is the bottom card, then we need to know if there are
        // kings that can be moved to the spot
        // If target is a king, then we need to see if there are any empty
        // tableaus to move it to
        if (pile_empty(&field->tableaus[i]))
            continue;
        src_card = pile_last_face_up_card(&field->tableaus[i]);
        // card_print(src_card);
        for (j = 0; j < NUM_TABLEAU; ++j) {
            if (pile_empty(&field->tableaus[j]) && src_card->rank == RANK_K)
                return false;

            dst_card = pile_top_card(&field->tableaus[j]);
            if (_tableau_move_valid(src_card, dst_card))
                return false;
        }

        for (j = 0; j < NUM_FOUNDATION; ++j) {
            dst_card = pile_top_card(&field->foundations[j]);
            if (_foundation_move_valid(src_card, dst_card))
                return false;
        }
    }

    if (!pile_empty(&field->stock)) {
        list_for_each_entry(src_card, &field->stock.list, list) {
            for (i = 0; i < NUM_TABLEAU; ++i) {
                dst_card = pile_top_card(&field->tableaus[i]);
                if (_tableau_move_valid(src_card, dst_card))
                    return false;
            }

            for (i = 0; i < NUM_FOUNDATION; ++i) {
                dst_card = pile_top_card(&field->foundations[i]);
                if (_tableau_move_valid(src_card, dst_card))
                    return false;
            }
        }
    }

    if (!pile_empty(&field->waste)) {
        list_for_each_entry(src_card, &field->waste.list, list) {
            for (i = 0; i < NUM_TABLEAU; ++i) {
                dst_card = pile_top_card(&field->tableaus[i]);
                if (_tableau_move_valid(src_card, dst_card))
                    return false;
            }

            for (i = 0; i < NUM_FOUNDATION; ++i) {
                dst_card = pile_top_card(&field->foundations[i]);
                if (_tableau_move_valid(src_card, dst_card))
                    return false;
            }
        }
    }

    return true;
}

bool
game_over(struct field *field)
{
    if (game_completion_check(field)) {
        printf("You beat the game!!!\n");
        return true;
    }

    if (dead_end_check(field)) {
        printf("Dead end\n");
        return true;
    }

    return false;
}

// TODO: Add help message for -h flag
