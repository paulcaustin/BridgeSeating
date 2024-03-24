### Bridge Seating

This is a pet project used in various ways.  The base problem
is how do I seat 24 people to play 8 rounds of bridge with
out two people ever being partners more than once.  In addition,
sitting at the same table as other people more than once is minimized.

I've used this problem to test various techniques for solving
large problems as well as learning new languages.

Here's a brief idea of what's included:

- gametree
    Unfinished attempt to do a lookahead strategy with
    an actual decision tree and pruning
- lookahead
    Originally the most successful.  Uses a crude recursive
    lookahead strategy which keeps the best n at each depth.
- showall
    Looks like a simple demonstration of using comb.c to
    list all possible game seatings.
- sim_anneal
    An implementation of Simulated Annealing
    See https://en.wikipedia.org/wiki/Simulated_annealing
- genetic
    A recent retry using something I read about genetic
    algorithms.  This seems to blow the doors off of
    all other attempts.
- python
    An implementation of the genetic algorithm in Python
- js
    An implementation of the genetic algorithm in Javascript
