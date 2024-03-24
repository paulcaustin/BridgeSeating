/*
 * This is a program to solve the Chamberlain Bridge Problem.  The
 * problem involves arranging a group of people to play several
 * bridge games (hands) with several tables.  The ultimate goal is
 * to devise a way for each person to play with each other person.
 * In an attempt to do this we have created a method of scoring an
 * arrangement.  We do this by counting the number of times somebody
 * plays with somebody they have already played with.  Thus we end
 * up making a table.  For example, for persons A, B, C, D, E, F, G,
 * and H, we might play these games:
 *
 *	ABCD EFGH
 *	ABEF CDGH
 *
 * which would result in a scoring table that looks like this:
 *
 *    A  B  C  D  E  F  G  H
 * A  .  2  1  1  1  1  0  0
 * B  2  .  1  1  1  1  0  0
 * C  1  1  .  2  0  0  1  1
 * D  1  1  2  .  0  0  1  1
 * E  1  1  0  0  .  1  1  1
 * F  1  1  0  0  1  .  1  1
 * G  0  0  1  1  1  1  .  2
 * H  0  0  1  1  1  1  2  .
 *
 * In my scoring scheme, this would be a score of 3.  That is
 * "3 times, a person played someone twice (or more)."  Of
 * course, symmetry applies so you could call it 6, but I'd
 * call that "redundant".  This is actually a pretty bad
 * looking table, but I don't think it could be improved upon.
 * Larger tables get exponentially more complicated.
 *
 * One setup which is incredibly complicated involves
 * 24 players, playing 8 games, with 6 tables playing each
 * game.  The solutions space of this problem is (I think)
 *	8C(4C24 * 4C20 * 4C16 * 4C12 * 4C8 * 4C4)
 * where mCn is Combinations of n things taken m at a time.
 * This is, roughly 3E+120 different possible configurations.
 * Clearly, we don't have time to "score" each of these.
 *
 * One method of finding a reasonable solution in this huge
 * space is called a Genetic Algorithm.  The important terms
 * in Genetic Algorithms are:
 *
 *	Chromosome
 *		This is the entire set of information that
 *		describes one possible solution to the problem.
 *	Population
 *		The collection of a fixed number of the most
 *		successful chromosomes.
 *	The Cross-over function
 *		Takes two "parent" chromosomes and randomly
 *		produces a "child" chromosome with
 *		characteristics of each "parent".
 *	The Mutation function
 *		Performs a random slight modification of a
 *		chromosome.
 *
 * Here is a brief summary of the algorithm:
 *
 *	Generate a "population" of random chromosomes through
 *	excessive mutation.  Always keep the population sorted
 *	by their performance (score).  Iteratively generate a
 *	new chromosome by choosing two well-performing parent
 *	chromosomes and performing a Cross-over function between
 *	them.  Use mutations until the new child chromosome is
 *	unique within the population.  Do this millions of times
 *	and hope the highest ranking chromosome gets better and
 *	better.
 *
 * The important things to a successful Genetic Algorithm are
 * the Cross-over function, the Mutation function, and the
 * Comparison for Uniqueness.  This particular implementation
 * uses rather sloppy functions.
 *
 *	The Cross-over function
 *		Since each chromosome contains seating config-
 *		urations for several games, the cross-over func-
 *		tion randomly chooses each seating configuration
 *		for a whole game from one parent or the other.
 *	The Mutation function
 *		A game and two seat positions are chosen.  The
 *		two positions are then swapped.  Later, it became
 *		necessary to require that the seating positions
 *		were from different tables.
 *	The Comparison for Uniqueness
 *		Each seat position in each game is compared to
 *		the same position and game in the other chromosome.
 *		Later, it became necessary to make the player
 *		position within a table not significant in the
 *		comparison.  Ideally, the order of the tables
 *		within a game, and the order of the games would
 *		also not be significant.
 */
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "bridge.h"

#define USAGE		"Usage: bridge [-o][-p population][-m maxiterations][-t termcount][-s swapcnt]\n"
char			*Pname;

CHROM			*popptr[MAXPOP] = { 0 };
CHROM			pop[MAXPOP] = { 0 };
CHROM			offspring = 0;
long			mutatecnt = 0;
long			crosscnt = 0;
long			insertcnt = 0;
long			pickcnt[MAXPOP] = { 0 };

int			verbose = 1;	/* default noisiness */


/*
 * Calculate the score of a chromosome and
 * record it within the chromosome.
 */
int evaluate(c)
CHROM			*c;
{
	static char	table[PLAYERS * PLAYERS];

	c->score = score_config(table,c);
	return(c->score);
}

/*
 * This program was based on a genetic algorithm
 * which made use of a "local optimization" to
 * make simple improvements to a chromosome
 * after generating it.  I couldn't figure out
 * a useful way to incorporate this feature.
 */
int localopt(chrom)
CHROM			*chrom;
{
#ifdef NOTUSED
	int		swaped = 0;
	int		p0, p1, p2, p3;
	int		cx;
	int		ec;
	unsigned char	*u;
	double		c, n;

	u = &chrom->p[0];
	ec = ncities - 2;
	for (cx = 0; cx <= ec; cx++) {
		p0 = u[(cx ? cx : ncities) - 1];
		p1 = u[cx];
		p2 = u[cx + 1];
		p3 = u[(cx == ec ? 0 : cx + 2)];
		c = dist[p0][p1] + dist[p2][p3];
		n = dist[p0][p2] + dist[p1][p3];
		if (n < c) {
			u[cx] = p2; u[cx + 1] = p1;
			swaped++;
		}
	}
	return(swaped);
#endif
}


/*
 * The Cross-over function: generate a child based
 * on the two parents using a random selection of
 * characteristics from each.
 */
int crossover(child, par1, par2, opt)
CHROM			*child;
CHROM			*par1, *par2;
int			opt;
{
	int		game;
	int		gamep;

	/*
	 * A very simple crossover algorithm.
	 * Either copy a game from parent one or parent two.
	 */
	for (game = 0; game < GAMES; game++) {
		gamep = game * PLAYERS;
		if ((rand() % 1000) >= 500)
			memcpy(&child->p[gamep], &par1->p[gamep], PLAYERS);
		else
			memcpy(&child->p[gamep], &par2->p[gamep], PLAYERS);
	}

	if (opt)
		localopt(child);
	
	evaluate(child);

	if (verbose) {
		printf("Cross-Over:\n");
		make_report(child, 1);
	}

	crosscnt++;
	return(child->score);
}


/*
 * The Mutate function: perform small modifications to
 * a chromosome by making some random changes.
 */
mutate(chrom, iter, opt)
CHROM			*chrom;
int			iter;		/* number of changes to make */
int			opt;
{
	unsigned char	*up, u;
	int		a, b, seat;

	up = &chrom->p[0];
	/*
	 * A simple mutation algorithm.
	 * Swap two players' seats in the same game.
	 */
	while (iter > 0) {
		a = rand() % PLAYERS;
		b = rand() % PLAYERS;
		if (a/4 != b/4) {	/* different tables */
			seat = (rand() % GAMES) * PLAYERS;
			u = up[seat + a];
			up[seat + a] = up[seat + b];
			up[seat + b] = u;
			iter--;
		}
	}

	if (opt)
		localopt(chrom);

	evaluate(chrom);

	if (verbose) {
		printf("Mutated:\n");
		make_report(chrom, 1);
	}

	mutatecnt++;
	return;
}


/*
 * The comparison function only to be used by the sort.
 */
int initcmp(pa, pb)
char			*pa, *pb;
{
	int		w;
	CHROM		*pca, *pcb;

	pca = *((CHROM **) pa);
	pcb = *((CHROM **) pb);
	w = pca->score - pcb->score;
	if (w > 0)
		return(1);
	else if (w < 0)
		return(-1);
	else
		return(0);
}


/*
 * Generate and sort a random population.
 */
void initpopulation(npop)
int			npop;
{
	CHROM		*chrom;
	int		cx;
	int		gx;
	int		px;

	if (verbose)
		printf("Initializing population...\n");
	memset((char *) &pop[0], 0, sizeof(pop));
	for (cx = 0; cx < npop; cx++) {
		chrom = &pop[cx];
		popptr[cx] = chrom;
		for (gx = 0; gx < GAMES; gx++)
			for (px = 0; px < PLAYERS; px++)
				chrom->p[gx * PLAYERS + px] = px;
		mutate(chrom, SEATS*2, 1);
	}
	if (verbose)
		printf("Sorting population...\n");
	qsort((void *) &popptr[0], npop, sizeof(popptr[0]),initcmp);
	if (verbose)
		printf("Done.\n");
	return;
}


/*
 * Choose a parent.  This is a random selection
 * from the population but it is weighted to favor
 * those near the top.  Unfortunately it seems to
 * favor the second highest much more than the
 * very highest, but I haven't fixed this yet.
 */
int pickparent(npop)
int			npop;
{
	int		accum;
	int		pop2;

	pop2 = npop + npop;
	accum = (rand() % pop2) - npop;
	accum += (rand() % pop2) - npop;
	accum += (rand() % pop2) - npop;
	if (accum < 0)
		accum = -accum;
	if (accum >= npop)
		accum %= npop;
	pickcnt[accum]++;
	return(accum);
}


/*
 * This is one iteration of the algorithm.
 * Choose two parents and create a child.  Try
 * to assure his uniqueness using mutations.
 * Insert him in the population.  Note that
 * the population is a fixed size and is
 * sorted so the poorest performing member
 * falls off the end (this may very well
 * be the new child).
 */
int evolveone(npop, maxswap, opt)
int			npop;
int			maxswap;
int			opt;
{
	int		p1, p2;
	int		px, ax;
	int		mutflag;
	int		duplicate;
	int		size;
	int		swap;
	int		w;
	CHROM		*chrom;

	for (;;) {
		p1 = pickparent(npop);
		p2 = pickparent(npop);
		if (p1 != p2)
			break;
	}

	if (verbose) {
		printf("Evolve parents %d and %d\n", p1, p2);
		make_report(popptr[p1], 1);
		make_report(popptr[p2], 1);
	}

	duplicate = 1;
	size = sizeof(CHROM);
	for (mutflag = 0; duplicate != 0 && mutflag < MUTATIONS; mutflag++) {
		if (mutflag) {
			swap = (rand() % maxswap) + 1;
			mutate(&offspring, swap, opt);
		} else
			crossover(&offspring, popptr[p1], popptr[p2], opt);
		duplicate = 0;
		for (px = 0; px < npop; px++) {
			w = offspring.score - popptr[px]->score;
			if (w < 0)
				break;
			if (w == 0 && 
			  chromcmp(&offspring, popptr[px]) == 0) {
				duplicate = 1;
				break;
			}
		}
	}

	if (duplicate == 0 && px < npop) {
		if (verbose)
			printf("Offspring inserted at position %d\n", px);
		chrom = popptr[npop-1];
		for (ax = npop-1; ax > px; ax--)
			popptr[ax] = popptr[ax-1];
		popptr[px] = chrom;
		memcpy((char *) chrom, (char *) &offspring, sizeof(offspring));
		insertcnt++;
		return(0);
	}
	return(-1);
}


/*
 * The main program entry point.
 * Parse the options given on the command line
 * and run the algorithm to completion.
 */
main(argc,argv)
int				argc;
char				*argv[];
{
	int			ch;
	extern int		optind;
	extern char		*optarg;
	int		npop = 100;
	int		maxswap = 10;
	long		maxiter = 10000;
	long		iter;
	long		termcount = 1000;
	long		bestctr;
	long		t0, t1, t2;
	int		w;
	int		opt = 0;
	int		best;

	if ((Pname = strrchr(argv[0], '/')) == NULL)
		Pname = argv[0];
	else
		Pname++;

	while ((ch = getopt(argc,argv,"vop:m:t:s:")) != EOF)
		switch (ch) {
		case 'v':
			verbose = !verbose;
			break;
		case 'o':
			opt = !opt;
			break;
		case 'p':
			npop = atol(optarg);
			break;
		case 'm':
			maxiter = atol(optarg);
			break;
		case 't':
			termcount = atol(optarg);
			break;
		case 's':
			maxswap = atol(optarg);
			break;
		default:
			usage();
		}

	time(&t0);

	if (npop > MAXPOP)
		npop = MAXPOP;
	
	srand(t0);
	initpopulation(npop);
	best = popptr[0]->score;
	bestctr = 0;

	iter = 0;
	if (verbose || 1) {
		printf("Population = %d\tMaxIter = %ld\n", npop, maxiter);
		printf("TermCount = %ld\tMaxSwap = %d\n", termcount, maxswap);
		printf("Opt = %d\n", opt);

		printf("Iteration Best Cnt  Score  Mutates   Cross-Ovr Inserts\n");
	}

	time(&t1);
	mutatecnt = crosscnt = insertcnt = 0;
	while (iter < maxiter && bestctr++ < termcount) {
		evolveone(npop, maxswap, opt);
		if (popptr[0]->score < best) {
			best = popptr[0]->score;
			bestctr = 0;
		}
		if (verbose == 0 && (iter % 1000) == 0) {
			printf("%9ld %9ld %6d %9ld %9ld %9ld\n",
			  iter, bestctr, popptr[0]->score, mutatecnt,
			  crosscnt, insertcnt);
			fflush(stdout);
			mutatecnt = crosscnt = insertcnt = 0;
		}
		iter++;
	}
	time(&t2);

	printf("\n%9ld %9ld %12d %9ld %9ld %9ld  *\n\n",
	  iter, bestctr, popptr[0]->score,
	  mutatecnt, crosscnt, insertcnt);
	
	printf("\nTotal Time = %ld seconds; Search Time = %ld seconds.\n",
	  t2 - t0, t2 - t1);
	if (iter >= maxiter)
		printf("Terminated due to Maximum Iteration Count\n");
	if (bestctr >= termcount)
		printf("Terminated due to Maximum Patience Reached\n");
	printf("Best Configuration:\n");
	make_report(popptr[0], 2);

	printf("\nParent Pick Counts\nRank\tCount\n");
	for (w = 0; w < npop; w++)
		printf("%4d\t%9ld\n", w, pickcnt[w]);
	return(0);
}


/*
 * Print out a chromosome.  Because I'm lazy this
 * actually serves 3 purposes:
 *	flag == 0
 *		really low level debug. just display
 *		the seating arrangements.
 *	flag == 1
 *		The verbose output.  Display the
 *		seating arrangements and the score.
 *	flag == 2
 *		The final output.  Display the seating
 *		arrangements, the score, and the table.
 */
make_report(c, flag)
CHROM			*c;
int			flag;
{
	int		player;
	char		table[PLAYERS * PLAYERS];

	for (player = 0; player < SEATS; player++) {
		putchar(c->p[player] + 'A');
		if (POSITION(player) == PPERT-1)
			if (TABLE(player) == TABLES-1)
				putchar('\n');
			else
				putchar(' ');
	}

	if (flag > 0)
		printf("\nScore = %d\n", score_config(table,c));

	if (flag > 1)
		print_table(table);
}


/*
 * Display on of our scoring tables.
 */
print_table(t)
char			*t;
{
	int		y;
	int		x;

	printf("  A B C D E F G H I J K L M N O P Q R S T U V W X\n");
	for (y = 0; y < PLAYERS; y++) {
		putchar('A' + y);
		for (x = 0; x < PLAYERS; x++) {
			if (x == y)
				printf(" .");
			else
				printf(" %c", '0'+t[y * PLAYERS + x]);
		}
		putchar('\n');
	}
}


/*
 * The actual scoring method.  This is fairly
 * well optimized and has been used by me many
 * times.  I know it works and yes, I know that
 * the TALLY macro is ugly.
 */
score_config(t,c)
char			*t;
CHROM			*c;
{
	int		i;
	int		score = 0;

	memset(t, 0, PLAYERS * PLAYERS);

	for (i = 0; i < SEATS; i += PPERT) {
		score += TALLY(t,c->p[i],c->p[i+1]);
		score += TALLY(t,c->p[i],c->p[i+2]);
		score += TALLY(t,c->p[i],c->p[i+3]);
		score += TALLY(t,c->p[i+1],c->p[i+2]);
		score += TALLY(t,c->p[i+1],c->p[i+3]);
		score += TALLY(t,c->p[i+2],c->p[i+3]);
	}
	return(score);
}


/*
 * Give the user a tiny hint as to how to run it.
 */
usage()
{
	fprintf(stderr,USAGE,Pname);
	exit(1);
}


/*
 * Rather than just comparing each seat position
 * try to take into account that position within
 * a table is not significant.  Neither is the
 * position of the table within a game or the game
 * within the chromosome but that's too hard for me.
 */
chromcmp(c1, c2)
CHROM			*c1;
CHROM			*c2;
{
	int		i;
	int		j;
	int		table;
	int		seat;

	for (table = 0; table * PPERT < SEATS; table++) {
		seat = table * PPERT;
		for (i = 0; i < PPERT; i++) {
			for (j = 0; j < PPERT; j++)
				if (c1->p[seat+i] == c2->p[seat+j])
					goto foundit;
			return(1);	/* different */
foundit:		;
		}
	}
	return(0);
}
