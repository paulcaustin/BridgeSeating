/*
 * This is a modified version with more restrictions.  It could
 * never score a zero for the full 6 table, 8 game problem.  A
 * big score is associated with repeated partners and a small
 * score is associated with repeated same-table positions.
 *
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
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "bridge.h"

#define USAGE		"Usage: bridge [opts]\n"
char			*Pname;

CHROM			*popptr[MAXPOP] = { 0 };
CHROM			pop[MAXPOP] = { 0 };
CHROM			offspring = { 0 };
long			mutatecnt = 0;
long			crosscnt = 0;
long			insertcnt = 0;
int			bestinsert = 999999;
int			bestmut;
long			pickcnt[MAXPOP] = { 0 };
char			*initfile = NULL;
struct sigaction	catch_action;
int			caught = 0;
int			justscore = 0;
int			players = DEF_PLAYERS;
int			games = DEF_GAMES;
int			keeptrying = MUTATIONS;
int			fancycompare = FANCY_COMPARE;
int			sortopt = DEF_SORT;
int			verbose = 0;	/* default noisiness */
int			mixeddoubles = 0;


/*
 * Calculate the score of a chromosome and
 * record it within the chromosome.
 */
int evaluate(c)
CHROM			*c;
{
	static char	table[PLAYERS * PLAYERS * 10];
	static char	table2[PLAYERS * PLAYERS];
	static char	table3[PLAYERS * PLAYERS / PPERT];

	c->score = score_config(table,table2,table3,c);
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
#ifdef USE_OPTIMIZE
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
	for (game = 0; game < games; game++) {
		gamep = game * players;
		if ((rand() % 1000) >= 500)
			memcpy(&child->p[gamep], &par1->p[gamep], players);
		else
			memcpy(&child->p[gamep], &par2->p[gamep], players);
	}

	if (opt)
		localopt(child);
	
	evaluate(child);

	if (verbose > 2) {
		printf("Cross-Over:\n");
		make_report(child, 1);
	}

	crosscnt++;
	return(child->score);
}

/*
 * Normalize a chromosome by sorting areas where order is not significant.
 */
int simplecmp(char *a, char *b) { return *a - *b; }
int tablecmp(char *a, char *b) { return memcmp(a, b, PPERT); }
int gamecmp(char *a, char *b) { return memcmp(a, b, players); }

swap(p1, p2)
unsigned char *p1, *p2;
{
	unsigned char tmp;
	tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

normalize(chrom)
CHROM			*chrom;
{
	int i;
	unsigned char *p1, *p2, tmp;

	switch (sortopt) {
	case 3:
		/* Position of tables within game not significant */
		for (i=0; i < games; i++) {
			qsort(&chrom->p[i * players], players / PPERT, PPERT, tablecmp);
		}
	case 2:
#ifdef SCORE_PARTNERS
		if (PPERT == 4)
		for (i=0; i < players*games; i += PPERT) {
			p1 = &chrom->p[i];
			/*
			 * Order team members
			 * Order teams
			 */
			if (p1[0] > p1[1]) swap(&p1[0], &p1[1]);
			if (p1[2] > p1[3]) swap(&p1[2], &p1[3]);
			if (p1[0] > p1[2]) {
				swap(&p1[0], &p1[2]);
				swap(&p1[1], &p1[3]);
			}
		}
#else
		/* Position within tables not significant */
		if (PPERT == 2)
			for (i=0; i < players*games; i += PPERT) {
				p1 = &chrom->p[i];
				p2 = &chrom->p[i+1];
				if (*p1 > *p2) {
					tmp = *p1;
					*p1 = *p2;
					*p2 = tmp;
				}
			}
		else
			for (i=0; i < players*games; i += PPERT) {
				qsort(&chrom->p[i], PPERT, 1, simplecmp);
			}
#endif
	case 1:
		/* Position of games within genome not significant */
#ifndef SCORE_TABLESTAY
		qsort(&chrom->p[0], games, players, gamecmp);
#endif
	default:
		return;
	}
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
		a = rand() % players;
		b = rand() % players;
		/*
		 * There could be plenty of logic here to
		 * make sure the swap is meaningful, but
		 * there isn't, that's left to the magic
		 * of the scoring algorithm.
		 */
		if (mixeddoubles && a%2 != b%2) {
			/* Always swap players of the same sex */
			b = (b + 1) % players;
		}
		seat = (rand() % (games-1)) * players + players;
		u = up[seat + a];
		up[seat + a] = up[seat + b];
		up[seat + b] = u;
		iter--;
	}

	if (opt)
		localopt(chrom);

	evaluate(chrom);

	if (verbose > 2) {
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
void initpopulation(npop, save)
int			npop;
int			save;
{
	CHROM		*chrom;
	int		cx;
	int		gx;
	int		px;
	FILE		*fp = NULL;

	if (verbose)
		printf("Initializing population...\n");
	if (initfile)
		if ((fp = fopen(initfile,"r")) == NULL) {
			perror(initfile);
			exit(1);
		}
	for (cx = save; cx < npop; cx++) {
		if (save) {
			chrom = popptr[cx];
		} else {
			chrom = &pop[cx];
			popptr[cx] = chrom;
		}
		if (fp) {
			if (readone(fp, chrom) == 0) {
				evaluate(chrom);
				if (verbose > 1)
					make_report(chrom, 2);
				continue;
			} else {
				fclose(fp);
				fp = NULL;
				if (verbose)
					printf("Read %d from %s\n",
					    cx, initfile);
			}
		}
		for (gx = 0; gx < games; gx++)
			for (px = 0; px < players; px++)
				chrom->p[gx * players + px] = px;
		mutate(chrom, players * games, 1);
		normalize(chrom);
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

	if (verbose > 2) {
		printf("Evolve parents %d and %d\n", p1, p2);
		make_report(popptr[p1], 1);
		make_report(popptr[p2], 1);
	}

	duplicate = 1;
	size = sizeof(CHROM);
	for (mutflag = 0; duplicate != 0 && mutflag < keeptrying; mutflag++) {
		if (mutflag && maxswap) {
			swap = (rand() % maxswap) + 1;
			mutate(&offspring, swap, opt);
		} else
			crossover(&offspring, popptr[p1], popptr[p2], opt);
		duplicate = 0;
#ifdef TRY_MUTATE_HARDER
		if (offspring.score > popptr[npop-1]->score) {
			duplicate = 1;	/* keep trying */
			continue;
		}
#endif
		normalize(&offspring);
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
		if (px < 2) save_best(&offspring);
		if (offspring.score < bestinsert) {
			bestinsert = offspring.score;
			bestmut = mutflag;
		}
		if (verbose && px < npop/5)
			printf("Offspring with score %d inserted at position %d\n", offspring.score, px);
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
	int		npop = DEF_NPOP;
	int		maxswap = DEF_MAXSWAP;
	long		disaster = DEF_DISASTER;
	long		maxiter = DEF_MAXITERATE;
	long		iter;
	long		termcount = DEF_TERMINATE;
	long		bestctr;
	long		t0, t1, t2;
	int		w;
	int		opt = 0;
	int		best;
	int		sig_catch();

	if ((Pname = strrchr(argv[0], '/')) == NULL)
		Pname = argv[0];
	else
		Pname++;

	while ((ch = getopt(argc,argv,"d:ck:g:n:ji:vop:m:t:s:S:x")) != EOF)
		switch (ch) {
		case 'i':
			initfile = optarg;
			break;
		case 'c':
			fancycompare = !fancycompare;
			break;
		case 'v':
			verbose++;
			break;
		case 'j':
			justscore = !justscore;
			break;
		case 'S':
			sortopt = atol(optarg);
			break;
		case 'k':
			keeptrying = atol(optarg);
			break;
		case 'g':
			games = atol(optarg);
			break;
		case 'n':
			players = atol(optarg);
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
		case 'd':
			disaster = atol(optarg);
			break;
		case 'x':
			mixeddoubles = !mixeddoubles;
			break;
		default:
			usage();
		}

	if (justscore) {
		FILE		*fp = NULL;
		if (!initfile) usage();
		if ((fp = fopen(initfile,"r")) == NULL) {
			perror(initfile);
			exit(1);
		}
		if (readone(fp, &pop[0]) != 0) {
			perror(initfile);
			exit(1);
		}
		evaluate(&pop[0]);
		make_report(&pop[0], 2);
		exit(0);
	}

	time(&t0);

	if (npop > MAXPOP)
		npop = MAXPOP;
	
	srand(t0);
	memset((char *) &pop[0], 0, sizeof(pop));
	initpopulation(npop, 0);
	best = popptr[0]->score;
	bestctr = 0;
	iter = 0;

	if (verbose || 1) {
		printf("Population = %d\tMaxIter = %ld\n", npop, maxiter);
		printf("TermCount = %ld\tMaxSwap = %d\n", termcount, maxswap);
		printf("Opt = %d\n", opt);

		printf("Iteration  Best Cnt   Scores    Mutates Inst Best BMut Swap\n");
	}

	catch_action.sa_handler = (void (*)()) sig_catch;
#ifdef OLDSIGSTUFF
	SIGINITSET(catch_action.sa_mask);
	SIGADDSET(catch_action.sa_mask, SIGINT);
	SIGADDSET(catch_action.sa_mask, SIGTERM);
	SIGADDSET(catch_action.sa_mask, SIGHUP);
#endif
	catch_action.sa_flags = 0;
	sigaction(SIGINT, &catch_action, (void *) NULL);
	sigaction(SIGTERM, &catch_action, (void *) NULL);
	sigaction(SIGHUP, &catch_action, (void *) NULL);


	time(&t1);
	mutatecnt = crosscnt = insertcnt = 0;
	while (iter < maxiter && bestctr++ < termcount && !caught) {
#ifdef NOT_RANDOM_DISASTER
		if (disaster > 0 && (iter % disaster) == 0) {
			initpopulation(npop, DISASTER_SAVE);
		}
#else
		if (disaster > 0 && (lrand48() % disaster) == 0) {
			initpopulation(npop, lrand48() % (npop-DISASTER_SAVE) + DISASTER_SAVE);
		}
#endif
		evolveone(npop, maxswap, opt);
		if (popptr[0]->score < best) {
			best = popptr[0]->score;
			bestctr = 0;
		}
		if (best == 0) break;
		if (verbose == 0 && (iter % REPORT_FREQ) == 0) {
			printf("%9ld %9ld %4d-%4d %9ld %4ld %4d %4d %d\n",
			  iter, bestctr, popptr[0]->score,
			  popptr[npop-1]->score, mutatecnt,
			  insertcnt,bestinsert,bestmut, maxswap);
			fflush(stdout);

			/*
			 * Automatically adjust maxswap
			 */
			if (insertcnt > npop/40 && maxswap > 5)
				maxswap = maxswap / 2 + 1;
			else if (insertcnt < npop/40 && maxswap < players*games)
				maxswap = maxswap * 2;

			mutatecnt = crosscnt = insertcnt = 0;
			bestinsert = 999999; bestmut = 0;
		}
		iter++;
	}
	time(&t2);

	printf("\n%9ld %9ld %4d-%4d %9ld %4ld  *\n\n",
	  iter, bestctr, popptr[0]->score, popptr[npop-1]->score,
	  mutatecnt, insertcnt);
	
	printf("\nTotal Time = %ld seconds; Search Time = %ld seconds.\n",
	  t2 - t0, t2 - t1);
	if (iter >= maxiter)
		printf("Terminated due to Maximum Iteration Count\n");
	if (bestctr >= termcount)
		printf("Terminated due to Maximum Patience Reached\n");
	printf("Best Configuration:\n");
	make_report(popptr[0], 2);

#ifdef SHOW_PICKCOUNTS
	printf("\nParent Pick Counts\nRank\tCount\n");
	for (w = 0; w < npop; w++)
		printf("%4d\t%9ld\n", w, pickcnt[w]);
#endif
	return(0);
}


getname(c)
char	c;
{
	char name = c;

	if (name < 26) {
		name += 'A';
	} else {
		name += 'a' - 26;
	}
	return(name);
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
	char		table[PLAYERS * PLAYERS * 10];
	char		table2[PLAYERS * PLAYERS];
	char		table3[PLAYERS * TABLES];
	char		name;

	for (player = 0; player < players * games; player++) {
		putchar(getname(c->p[player]));
		if (player % PPERT == PPERT-1)
			if ((player % players) == players-1)
				putchar('\n');
			else
				putchar(' ');
	}

	if (flag > 0) {
		printf("\nStored Score = %d\n", c->score);
		printf("\nScore = %d\n", score_config(table,table2,table3,c));
		printf("Old Score = %d\n", c->old_score);
	}

	if (flag > 1) {
#ifdef SHOW_EXTRA
		print_table(table+players*players);
		print_table(table+players*players*2);
		print_table(table+players*players*3);
		print_table(table+players*players*4);
		print_table(table+players*players*5);
		print_table(table+players*players*6);
		print_table(table+players*players*7);
		print_table(table+players*players*8);
		print_table(table+players*players*9);
#endif

#ifdef SHOW_SEATING
		printf("Bad seating\n");
		print_table(table);
#endif

#ifdef SCORE_PARTNERS
		printf("\nBad partnerings\n");
		print_table(table2);
#endif
#ifdef SHOW_TABLING
		{	int i,j;
			printf("Bad tabling\n");
			for (i=0; i<TABLES; i++) {
				printf("Table %d: ", i+1);
				for (j=0; j<players; j++) {
					printf("%d ",NOTALLY(table3,i,j));
				}
				printf("\n");
			}
		}
#endif
	}
}


/*
 * Display one of our scoring tables.
 */
print_table(t)
char			*t;
{
	int		y;
	int		x;
	int		player;


	putchar(' ');
	putchar(' ');
	for (player = 0; player < players; player++) {
		putchar(getname(player));
		putchar(' ');
	}
	putchar('\n');

	for (y = 0; y < players; y++) {
		putchar(getname(y));
		for (x = 0; x < players; x++) {
			if (x == y)
				printf(" .");
			else
				printf(" %c", '0'+NOTALLY(t,y,x));
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
score_config(t,t2,t3,c)
char			*t;
char			*t2;
char			*t3;
CHROM			*c;
{
	int		i, j, k;
	int		score = 0;
	int		old_score = 0;
	int		table;
	int		day;
	int		game;
	register unsigned char *p, *p2;

#ifdef SCORE_TABLE1
	memset(t, 0, PLAYERS * PLAYERS * 10);
#endif
#ifdef SCORE_TABLE2
	memset(t2, 0, PLAYERS * PLAYERS);
#endif
#ifdef SCORE_TABLE3
	memset(t3, 0, PLAYERS * TABLES);
#endif

	for (game = 0; game < games; game++)
	for (i = 0; i < players; i += PPERT) {
		/*
		 * Each iteration is one table of one game.
		 * players MUST be divisible by PPERT.
		 */
		p = &c->p[game*players+i];
		table = i / PPERT;

#ifdef SCORE_TABLESTAY
		if (game > 1) {
			p2 = &c->p[game*players-players+i];
			for (j = 0; j < PPERT; j++)
				for (k = 0; k < PPERT; k++)
					if (p[j] == p2[k]) score += BAD_TABLE;
		}
#endif

		/* if (i >= PPERT) continue; temp: only one table plays */

		/*
		 * repeated play-with's
		 */
#ifdef SCORE_PLAYWITH
		for (j = 0; j < PPERT; j++) {
			for (k = j+1; k < PPERT; k++) {
				old_score += NOTALLY(t, p[j], p[k]);
				score += BAD_WITH*TALLY2(t, p[j], p[k]);
			}
		}
#endif

#ifdef SCORE_PARTNERS
		/*
		 * repeated partnering, big no-no.
		 */
		score += BAD_PARTNER*TALLY2(t2, p[0], p[1]);
		if (PPERT == 4) {
			score += BAD_PARTNER*TALLY2(t2, p[2], p[3]);
		}
#endif
#ifdef SCORE_SOMETHING
		day = i/players/5;
		score += BAD_PARTNER*TALLY(t+day*players*players, p[0], p[1]);
#endif

#ifdef SCORE_GENDER
		/*
		 * Implement mixed doubles.  odd=male, even=female
		 * This is an old implementation.  Use option x
		 * for mixeddoubles instead.
		 */
		if (p[0] % 2 == p[1] % 2) {
			score += 1000;
		}
		if (PPERT == 4 && p[2] % 2 == p[3] % 2) {
			score += 1000;
		}
#endif

		/*
		 * repeated tabling
		 */
#ifdef SCORE_TABLING
		for (j = 0; j < PPERT; j++)
			score += BAD_TABLE * TALLY(t3, table, p[j]);
#endif
#ifdef DEBUG_SCORE
		printf("Scored game %d, table %d: %d, %d\n"
			, game, table, old_score, score);
#endif
	}
	c->old_score = old_score;
	return(score);
}


/*
 * Give the user a tiny hint as to how to run it.
 */
usage()
{
	fprintf(stderr,USAGE,Pname);
	fprintf(stderr,"-t n\tTerminate after n iterations with no improvement\n");
	fprintf(stderr,"-m n\tTerminate after n iterations\n");
	fprintf(stderr,"-s n\tSwap n seats on each offspring try\n");
	fprintf(stderr,"-k n\tTry each offspring n times\n");
	fprintf(stderr,"-p n\tKeep a population of the n most successful setups\n");
	fprintf(stderr,"-i fn\tRead a few setups from the file named fn\n");
	fprintf(stderr,"-c\tUse a fancy compare to eliminate symmetric duplicate\n");
	fprintf(stderr,"-v\tBe verbose\n");
	fprintf(stderr,"-j\tDon't iterate at all, just give a score\n");
	fprintf(stderr,"-s n\t0: No sort, 1: sort games, 2: sort tables, 3: sort players\n");
	fprintf(stderr,"-g n\tNumber of games\n");
	fprintf(stderr,"-n n\tNumber of players\n");
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
	void		*gamestart, *memchr();
	unsigned char	p1, p2;

	for (table = 0; table * PPERT < players * games; table++) {
		seat = table * PPERT;
		if (fancycompare) {
			gamestart = (void *) &c2->p[seat/players*players];
			p1 = memchr(gamestart, c1->p[seat], players) - gamestart;
			p2 = memchr(gamestart, c1->p[seat+1], players) - gamestart;
			if (p1/PPERT != p2/PPERT) return 1;
		} else {
			for (i = 0; i < PPERT; i++) {
				p1 = c1->p[seat+i];
				for (j = 0; j < PPERT; j++)
					if (p1 == c2->p[seat+j])
						goto foundit;
				return(1);	/* different */
	foundit:		;
			}
		}
	}
	return(0);
}


/*
 * Calculate the score of a chromosome and
 * record it within the chromosome.
 */
int readone(in,c)
FILE			*in;
CHROM			*c;
{
	static char	buf[BUFSIZ];
	int		game = 0;
	int		player = 0;
	int		i;
	int		len;
	char		ch;

	while (fgets(buf,BUFSIZ-1,in)) {
		if (buf[0] == '\n' || buf[0] == '#') continue;
		if (verbose > 1)
			printf("%d: %s", game, buf);
		len = strlen(buf)-1;
		i = 0;
		for (player = 0; i < len && player < players; i++) {
			ch = buf[i];
			if (ch == '\t' || ch == ' ') continue;
			if (ch >= 'a') ch += 'A' - 'a' + 26;
			if (ch >= 'A') {
				c->p[game*players+player++] = ch - 'A';
			} else {
				fprintf(stderr, "Bad char: %x\n",ch);
				exit(1);
			}
		}
		if (++game >= games)
			return(0);
	}
	return(1);
}

save_best(c)
CHROM			*c;
{
	int		player;
	char		name;
	FILE		*fp;

	if ((fp = fopen("best","w")) == NULL) {
		perror("best");
		exit(1);
	}

	fprintf(fp, "# Score = %d, old_score=%d\n", c->score, c->old_score);
	for (player = 0; player < players * games; player++) {
		fputc(getname(c->p[player]), fp);
		if (player % PPERT == PPERT-1)
			if ((player % players) == players-1)
				fputc('\n', fp);
			else
				fputc(' ', fp);
	}

	fclose(fp);
}



sig_catch(signal, code, scp)
int			signal;
int			code;
struct sigcontext	*scp;
{
	caught++;
}
