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
int			players = 24;
int			games = 23;
int			keeptrying = 3;
int			fancycompare = 0;
int			sortopt = 0;
int			verbose = 0;	/* default noisiness */


/*
 * Calculate the score of a chromosome and
 * record it within the chromosome.
 */
int evaluate(c)
CHROM			*c;
{
	static char	table[PLAYERS * PLAYERS * 10];
	static char	table2[PLAYERS * PLAYERS];
	static char	table3[PLAYERS * TABLES];

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

	if (verbose) {
		printf("Cross-Over:\n");
		make_report(child, 1);
	}

	crosscnt++;
	return(child->score);
}

/*
 * Normalize a chromosome by sorting areas where order is not significant.
 */
int tablecmp(char *a, char *b) { return memcmp(a, b, PPERT); }
int gamecmp(char *a, char *b) { return memcmp(a, b, players); }

normalize(chrom)
CHROM			*chrom;
{
	int i;
	unsigned char *p1, *p2, tmp;

	switch (sortopt) {
	case 3:
		/* Position within tables not significant */
		for (i=0; i < players*games; i += PPERT) {
			if (PPERT == 2) {
				p1 = &chrom->p[i];
				p2 = &chrom->p[i+1];
				if (*p1 > *p2) {
					tmp = *p1;
					*p1 = *p2;
					*p2 = tmp;
				}
			} else {
				/* not implemented */
				break;
			}
		}
	case 2:
			
		/* Position of tables within game not significant */
		for (i=0; i < games; i++) {
			qsort(&chrom->p[i * players], players / PPERT, PPERT, tablecmp);
		}
	case 1:
		/* Position of games within genome not significant */
		qsort(&chrom->p[0], games, players, gamecmp);
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
		seat = (rand() % (games-1)) * players + players;
		u = up[seat + a];
		up[seat + a] = up[seat + b];
		up[seat + b] = u;
		iter--;
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
	FILE		*fp = NULL;

	if (verbose)
		printf("Initializing population...\n");
	memset((char *) &pop[0], 0, sizeof(pop));
	if (initfile)
		if ((fp = fopen(initfile,"r")) == NULL) {
			perror(initfile);
			exit(1);
		}
	for (cx = 0; cx < npop; cx++) {
		chrom = &pop[cx];
		popptr[cx] = chrom;
		if (fp) {
			if (readone(fp, chrom) == 0) {
				evaluate(chrom);
				if (verbose)
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

	if (verbose) {
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
#ifndef GIVE_UP_QUICK
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
		if (offspring.score < bestinsert) {
			bestinsert = offspring.score;
			bestmut = mutflag;
		}
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
	long		maxiter = 100000000;
	long		iter;
	long		termcount = 1000000;
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

	while ((ch = getopt(argc,argv,"ck:g:n:ji:vop:m:t:s:S:")) != EOF)
		switch (ch) {
		case 'i':
			initfile = optarg;
			break;
		case 'c':
			fancycompare = !fancycompare;
			break;
		case 'v':
			verbose = !verbose;
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
	if (justscore) {
		make_report(popptr[0], 2);
		exit(0);
	}

	iter = 0;
	if (verbose || 1) {
		printf("Population = %d\tMaxIter = %ld\n", npop, maxiter);
		printf("TermCount = %ld\tMaxSwap = %d\n", termcount, maxswap);
		printf("Opt = %d\n", opt);

		printf("Iteration Best Cnt  Score   Mutates   Cross-Ovr Inserts\n");
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
		evolveone(npop, maxswap, opt);
		if (popptr[0]->score < best) {
			best = popptr[0]->score;
			bestctr = 0;
		}
		if (best == 0) break;
		if (verbose == 0 && (iter % 10000) == 0) {
			printf("%9ld %9ld %3d,%3d %9ld %9ld %9ld %3d %3d %d\n",
			  iter, bestctr, popptr[0]->score,
			  popptr[0]->old_score, mutatecnt,
			  crosscnt, insertcnt,bestinsert,bestmut, maxswap);
			fflush(stdout);

			/*
			 * Adjust maxswap
			 */
			if (insertcnt > 5 && maxswap > 5)
				maxswap = maxswap / 2 + 1;
			else if (!insertcnt && maxswap < players*games/2)
				maxswap++;

			mutatecnt = crosscnt = insertcnt = 0;
			bestinsert = 999999; bestmut = 0;
		}
		iter++;
	}
	time(&t2);

	printf("\n%9ld %9ld %3d,%3d %9ld %9ld %9ld  *\n\n",
	  iter, bestctr, popptr[0]->score, popptr[0]->old_score,
	  mutatecnt, crosscnt, insertcnt);
	
	printf("\nTotal Time = %ld seconds; Search Time = %ld seconds.\n",
	  t2 - t0, t2 - t1);
	if (iter >= maxiter)
		printf("Terminated due to Maximum Iteration Count\n");
	if (bestctr >= termcount)
		printf("Terminated due to Maximum Patience Reached\n");
	printf("Best Configuration:\n");
	make_report(popptr[0], 2);

#ifndef PICKCOUNTS
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
		if (player % 2)
			if ((player % players) == players-1)
				putchar('\n');
			else
				putchar(' ');
	}

	if (flag > 0) {
		printf("\nScore = %d\n", score_config(table,table2,table3,c));
		printf("Old Score = %d\n", c->old_score);
	}

	if (flag > 1) {
#ifdef NOTUSED
		print_table(table);
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
		/*
		printf("Bad seating\n");
		*/
		printf("Bad partnerings\n");
		print_table(table2);
#ifdef NOTUSED
		{	int i,j;
			printf("Bad tabling\n");
			for (i=0; i<TABLES; i++) {
				for (j=0; j<players; j++) {
					printf("%d ",table3[j*TABLES+i]);
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
				printf(" %c", '0'+t[y * players + x]);
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
	int		i;
	int		score = 0;
	int		old_score = 0;
	int		table;
	int		day;
	register unsigned char *p;

#ifdef NOTUSED
	memset(t, 0, PLAYERS * PLAYERS * 10);
	memset(t3, 0, PLAYERS * TABLES);
#endif
	memset(t2, 0, PLAYERS * PLAYERS);

	for (i = 0; i < players * games; i += PPERT) {
		p = &c->p[i];
		/*
		 * repeated play-with's
		 */
#ifdef NOTUSED
		old_score += TALLY(t,p[0],p[1]);
		old_score += TALLY(t,p[0],p[2]);
		old_score += TALLY(t,p[0],p[3]);
		old_score += TALLY(t,p[1],p[2]);
		old_score += TALLY(t,p[1],p[3]);
		old_score += TALLY(t,p[2],p[3]);
#endif

		/*
		 * repeated partnering, big no-no.
		 */
		score += BAD_PARTNER*TALLY(t2,p[0],p[1]);
#ifdef NOTUSED
		day = i/players/5;
		score += BAD_PARTNER*TALLY(t+day*players*players,p[0],p[1]);
#endif

		/*
		 * repeated tabling
		 */
#ifdef NOTUSED
		table = (i/PPERT)%TABLES;
		score += 2 * t3[p[0]*TABLES+table]++;
		score += 2 * t3[p[1]*TABLES+table]++;
		score += 2 * t3[p[2]*TABLES+table]++;
		score += 2 * t3[p[3]*TABLES+table]++;
#endif
	}
	c->old_score = old_score;
	return(score + old_score * 5);
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
		if (verbose)
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


sig_catch(signal, code, scp)
int			signal;
int			code;
struct sigcontext	*scp;
{
	caught++;
}
