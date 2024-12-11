#Welcome to Klondike in C!

A simple command line Klondike Solitaire game.

Run the makefile to compile the "klondike" executable.

To play, type which card you would like to move followed by a space and then the
position you would like to move it to.

The card must be indicated by the first letter of the card rank followed by the
first letter of the card suit.

The position must be indicated by the first letter of the name of the position
(a 't' for tableau and an 'f' for foundation) followed by which foundation.

stock/waste: XX    5♦    foundations: XX  XX  XX  XX
tableaus:
                        XX
                    XX  XX
                XX  XX  XX
            XX  XX  XX  XX
        XX  XX  XX  XX  XX
    XX  XX  XX  XX  XX  XX
K♦  7♦  2♠  X♦  A♠  8♠  4♥

For example, to move the Ace of Spades to the first foundation, you should type
"as f1" to move it.

You can type "deal" followed by a space and any letter to deal a card.
You can type "undo" followed by a space and any letter to undo.

A better command line interface, and then a graphical interfaced are in the
works.
