/*
 * This is a modified version with more restrictions.  It could
 * never score a zero for the full 4 table, 6 game problem.  A
 * big score is associated with repeated partners and a small
 * score is associated with repeated same-table positions.
 */
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "bridge.h"

#define USAGE		"Usage: bridge [-o][-p population][-m maxiterations][-t termcount][-s swapcnt]\n"
char			*Pname;

CHROM			*popptr[MAXPOP] = { 0 };
CHROM			pop[MAXPOP] = { 0 };
CHROM			offspring = { 0 };
long			mutatecnt = 0;
long			crosscnt = 0;
long			insertcnt = 0;
int			bestinsert = 999;
int			bestmut;
long			pickcnt[MAXPOP] = { 0 };
char			*initfile = NULL;
struct sigaction	catch_action;
int			caught = 0;
int			justscore = 0;

int			verbose = 0;	/* default noisiness */


/*
 * Calculate the score of a chromosome and
 * record it within the chromosome.
 */
int evaluate(c)
CHROM			*c;
{
	static char	table[PLAYERS * PLAYERS];
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
		seat = (rand() % GAMES) * PLAYERS;
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
		for (gx = 0; gx < GAMES; gx++)
			for (px = 0; px < PLAYERS; px++)
				chrom->p[gx * PLAYERS + px] = px;
		mutate(chrom, SEATS*4, 1);
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
#ifdef KEEPTRYING
		if (offspring.score > popptr[npop-1]->score) {
			duplicate = 1;	/* keep trying */
			continue;
		}
#endif
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

	while ((ch = getopt(argc,argv,"ji:vop:m:t:s:")) != EOF)
		switch (ch) {
		case 'i':
			initfile = optarg;
			break;
		case 'v':
			verbose = !verbose;
			break;
		case 'j':
			justscore = !justscore;
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
		if (verbose == 0 && (iter % 10000) == 0) {
			printf("%9ld %9ld %3d,%3d %9ld %9ld %9ld %3d %3d\n",
			  iter, bestctr, popptr[0]->score,
			  popptr[0]->old_score, mutatecnt,
			  crosscnt, insertcnt,bestinsert,bestmut);
			fflush(stdout);
			mutatecnt = crosscnt = insertcnt = 0;
			bestinsert = 999; bestmut = 0;
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

#ifdef PICKCOUNTS
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
		name += '1' - 26;
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
	char		table[PLAYERS * PLAYERS];
	char		table2[PLAYERS * PLAYERS];
	char		table3[PLAYERS * TABLES];
	char		name;

	for (player = 0; player < SEATS; player++) {
		putchar(getname(c->p[player]));
		if (POSITION(player) == PPERT-1)
			if (TABLE(player) == TABLES-1)
				putchar('\n');
			else
				putchar(' ');
	}

	if (flag > 0) {
		printf("\nScore = %d\n", score_config(table,table2,table3,c));
		printf("Old Score = %d\n", c->old_score);
	}

	if (flag > 1) {
		printf("Bad seating\n");
		print_table(table);
		printf("Bad partnerings\n");
		print_table(table2);
		{	int i,j;
			printf("Bad tabling\n");
			for (i=0; i<TABLES; i++) {
				for (j=0; j<PLAYERS; j++) {
					printf("%d ",table3[j*TABLES+i]);
				}
				printf("\n");
			}
		}
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

	printf("  A B C D E F G H I J K L M N O P Q R S T U V W X\n");
	for (y = 0; y < PLAYERS; y++) {
		putchar(getname(y));
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
score_config(t,t2,t3,c)
char			*t;
char			*t2;
char			*t3;
CHROM			*c;
{
	int		i;
	int		s;
	int		score = 0;
	int		old_score = 0;
	int		table;
	register unsigned char *p;

	memset(t, 0, PLAYERS * PLAYERS);
	memset(t2, 0, PLAYERS * PLAYERS);
	memset(t3, 0, PLAYERS * TABLES);

	for (i = 0; i < SEATS; i += PPERT) {
		p = &c->p[i];
		/*
		 * repeated play-with's
		 */
		old_score += TALLY(t,p[0],p[1]);
		old_score += TALLY(t,p[0],p[2]);
		old_score += TALLY(t,p[0],p[3]);
		old_score += TALLY(t,p[1],p[2]);
		old_score += TALLY(t,p[1],p[3]);
		old_score += TALLY(t,p[2],p[3]);

		/*
		 * repeated partnering, big no-no.
		 */
		score += BAD_PARTNER*TALLY(t2,p[0],p[1]);
		score += BAD_PARTNER*TALLY(t2,p[2],p[3]);

#ifdef GENDER_COUNTS
		/*
		 * Implement mixed doubles.  odd=male, even=female
		 */
		if (p[0] % 2 == p[1] % 2) {
			score += 10*BAD_PARTNER;
		}
		if (p[2] % 2 == p[3] % 2) {
			score += 10*BAD_PARTNER;
		}
#endif

		/*
		 * repeated tabling
		 * exponential no-no.
		 */
		table = (i/PPERT)%TABLES;
		s = t3[p[0]*TABLES+table]++;
		score += BAD_TABLE * s * s;
		s = t3[p[1]*TABLES+table]++;
		score += BAD_TABLE * s * s;
		s = t3[p[2]*TABLES+table]++;
		score += BAD_TABLE * s * s;
		s = t3[p[3]*TABLES+table]++;
		score += BAD_TABLE * s * s;
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
		for (player = 0; i < len && player < PLAYERS; i++) {
			ch = buf[i];
			if (ch == '\t' || ch == ' ') continue;
			if (ch >= 'a') ch -= 'a' - 'A';
			if (ch >= 'A' && ch <= 'X') {
				c->p[game*PLAYERS+player++] = ch - 'A';
			} else {
				fprintf(stderr, "Bad char: %x\n",ch);
				exit(1);
			}
		}
		if (++game >= GAMES)
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
