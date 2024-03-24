/*
 * Determine good seating arrangement for a bridge tournament
 *
 * Don't sit at the same table more than twice
 * Don't partner with anyone more than once
 * Don't play against anyone more than twice
 */
#include <stdio.h>
#include <signal.h>
#include <math.h>

#define ALARM		120			/* # seconds between prints */

#define TABLES		4
#define PLAYERS		16
#define GAMES		8
#define PPERT		(PLAYERS / TABLES)

/* the table is arranged 0,2,1,3 for some optimization in first_player() */
#define PART(nn)	((nn) ^ 1)		/* guy accross the table */
#define OPP1(nn)	((nn) ^ 2)		/* guy to one side */
#define OPP2(nn)	((nn) ^ 3)		/* guy to the other side */
#define TABLE(nn)	((nn) >> 2)		/* which table */
#define PERS(xx,yy)	(((xx)<<2)|(yy))

#define MAX(x,y)	(((x) < (y)) ? (y) : (x))
#define RAND(x)		(rand() % x)

#ifdef DEBUG
int verbose;
#endif

int		setup[GAMES][PLAYERS];
int		best[GAMES][PLAYERS];
int		last[GAMES][PLAYERS];
int		caught = 0;
long		scored = 0;

/* for anneal */
#define BOOLEAN		int
#define TRUE		1
#define FALSE		0
#define FORWARD		1
#define BACK		0

#define ABS(x)		(((x) < 0) ? (-(x)) : (x))

float		ratios[10][2];
float		sigma, half_sigma;
long		temp, best_temp, t_low, t_min, step = 0, seed;
long		score, best_score, tot_succ, tot_fail, scor, scale;
long		nsucc, nfail, incount, outcount, bscore, wscore, uphill;
long		max_change, totscore, tot2score, avg_score, score_lim;
int		lim_incount, lim_outcount, lim_first, ultimate_lim;
int		min_succ, exg_cutoff;
BOOLEAN		frozen = FALSE;
BOOLEAN		quench = FALSE;
BOOLEAN		final = FALSE;
/* end of anneal's stuff */


main(argc,argv)
int argc;
char *argv[];
{
	if (argc < 2) {
		fprintf(stderr,"Usage: anneal paramfile\n");
		exit(2);
	}
	catch();
	initialize(argv[1]);
	anneal();
}


#define READ(fmt,var)	{						\
	fgets(buf,BUFSIZ-1,in);						\
	if (sscanf(buf,fmt,&var) != 1) {				\
		fprintf(stderr,"%s: bad value for var \n",name);	\
		exit(1);						\
	} else {							\
		fprintf(stderr,"read var = ");				\
		fprintf(stderr,fmt,var);				\
		fprintf(stderr,"\n");					\
	} }

initialize(name)
char		*name;
{
	char		buf[BUFSIZ];
	FILE		*in;
	int		i;
	long		time();

	if ((in = fopen(name,"r")) == NULL) {
		perror(name);
		exit(1);
	}
	READ("%ld", temp);
	READ("%ld", t_low);
	READ("%ld", t_min);
	READ("%ld", avg_score);
	READ("%ld", scale);
	READ("%f", sigma);
	READ("%d", exg_cutoff);
	READ("%d", min_succ);
	READ("%d", lim_incount);
	READ("%d", lim_outcount);
	READ("%d", lim_first);
	READ("%d", ultimate_lim);
	seed = time((long *) 0) + getpid();
	for (i=0; i<10; i++) {
		fgets(buf, BUFSIZ-1, in);
		if (sscanf(buf, "%f %f", &ratios[i][0], &ratios[i][1]) != 2) {
			fprintf(stderr, "%s: bad value for ratios %d\n", name, i);
			exit(1);
		}
	}
	score = best_score = 1000000;
	score_lim = 1000;
	first_hand();
}


first_hand()
{
	int		hand;
	int		p;

	for (hand=0; hand<GAMES; hand++)
		for (p=0; p<PLAYERS; p++)
			setup[hand][p] = p;
}


copy(to, from)
int		to[GAMES][PLAYERS];
int		from[GAMES][PLAYERS];
{
	register int	hand;
	register int	p;

	for (hand=0; hand<GAMES; hand++)
		for (p=0; p<PLAYERS; p++)
			to[hand][p] = from[hand][p];
}


/*
 * Don't sit at the same table more than twice
 * Don't partner with anyone more than once
 * Don't play against anyone more than twice
 */
get_score()
{
	int		s = 0;
	int		hand;
	register int	i;		/* counter */
	register int	p;		/* player */
	int		table[PLAYERS][TABLES];
	int		part[PLAYERS][PLAYERS];
	int		with[PLAYERS][PLAYERS];

	scored++;
	for (p=0; p<PLAYERS; p++) {
		for (i=0; i<TABLES; i++)
			table[p][i] = 0;
		for (i=0; i<PLAYERS; i++) {
			part[p][i] = 0;
			with[p][i] = 0;
		}
	}
	for (hand=0; hand<GAMES; hand++)
		for (i=0; i<PLAYERS; i++) {
			p = setup[hand][i];
			if (table[p][TABLE(i)]++ >= 2)
				s += 10;		/* at this table */
			if (part[p][setup[hand][PART(i)]]++)
				s += 15;		/* with this partner */
			if (with[p][setup[hand][PART(i)]]++ >= 2)
				s += 7;			/* with partner */
			if (with[p][setup[hand][OPP1(i)]]++ >= 2)
				s += 5;			/* with opponent 1 */
			if (with[p][setup[hand][OPP2(i)]]++ >= 2)
				s += 5;			/* with opponent 2 */
		}
	return(s);
}


show_setup(array)
int		array[GAMES][PLAYERS];
{
	int		j;
	register int	i;

	for (j=0; j<GAMES; j++) {
		printf("Hand %d:",j);
		for (i=0; i<PLAYERS; i++) {
			if ((i & 3) == 0)	/* quick for i % PPERT == 0 */
				putchar('\t');
			printf(" %d",array[j][i]);
		}
		putchar('\n');
	}
	putchar('\n');
}


catch()
{
	caught = 1;
	signal(SIGALRM,catch);
	alarm(ALARM);
}

init()
{
	nsucc = nfail = incount = outcount = wscore = max_change = uphill = 0;
	bscore = 1000000;
	totscore = tot2score = 0.0;
	if (temp < t_min)
		frozen = TRUE;
}


long rand()
{
	register long	k;

	k = seed / 52774;
	seed = 40692 * (seed - k*52774) - k*3791;
	if (seed < 0)
		seed += 2147483399;
	
	return(seed);
}


reconfigure(dir)
int		dir;
{
	int		hand;
	int		num;
	int		tmp;
	int		start;
	register int	i, j;

	if (dir == BACK) {
		copy(setup, last);		/* setup <- last */
		return(0);
	}
	copy(last, setup);			/* last <- setup */

	if (RAND(1000) < exg_cutoff) {		/* giant step */
		hand = RAND(GAMES);		/* rotate tables */
		num = RAND(TABLES-1)+1;		/* # tables to rotate */
		start = RAND(TABLES);		/* starting with # */
		for (i=0; i<PPERT; i++) {
			tmp = setup[hand][start*PPERT+i];
			for (j=start; j<start+num; j++)
				setup[hand][(j*PPERT+i)%PLAYERS] =
					setup[hand][((j+1)*PPERT+i)%PLAYERS];
			setup[hand][(j*PPERT+i)%PLAYERS] = tmp;
		}
	} else {				/* baby step */
		hand = RAND(GAMES);		/* swap two players */
		num = RAND(PLAYERS);
		start = RAND(PLAYERS);
		tmp = setup[hand][start];
		setup[hand][start] = setup[hand][num];
		setup[hand][num] = tmp;
	}
}


report()
{
	tot_succ += nsucc;
	tot_fail += nfail;
	printf("nsucc = %ld\tnfail = %ld\ttot_succ = %ld\ttot_fail = %ld",
		nsucc, nfail, tot_succ, tot_fail);
	printf("uphill = %ld\tscore = %ld\t score_lim = %ld\n",
		uphill, score, score_lim);

	if (quench)
		printf("Now Quenching\n");
	
	if (frozen)
		printf("Final report ---------------------");
	else
		printf("Report after %ld steps, %ld tries\n", step, scored);
	
	printf("\nBest score = %ld, Best Temperature = %ld\n",
		best_score, best_temp);
	show_setup(best);

	if (!frozen) {
		printf("\nCurrent score = %ld, Temperature = %ld\n",
			score, temp);
		show_setup(setup);
	}
}


new_temp()
{
	static int	next = 0;

	printf("New temperature: temp = %ld\tborder = %f\n", temp, ratios[next][0]);
	if (temp < ratios[next][0])
		next++;
	
	temp = ratios[next][1]*temp;
}


update()
{
	float ascore, s;

	if (nsucc > 0) {
		ascore = ((float) totscore) / nsucc;
		avg_score = ascore + scale;
		s = tot2score/nsucc - ascore*ascore;
		if (s > 0) {
			sigma = sqrt(s);
			half_sigma = sigma / 2;
			score_lim = avg_score - half_sigma;
		}
	}

	if ((temp < t_low) && ((bscore-wscore) == max_change))
		frozen = TRUE;
}


anneal()
{
	while(!frozen) {
		try_temp();
		update();
		if (caught || frozen) {
			report();
			caught = 0;
		}
		step++;
		if (temp > 0)
			new_temp();
	}
	temp = 0;
	quench = TRUE;
	try_temp();
	update();
	report();
	step++;
	copy(setup,best);		/* setup <- best */
	try_temp();
	update();
	report();
}


check_eq()
{
	if ((nsucc+nfail) > ultimate_lim)
		return(1);
	if (nsucc >= min_succ) {
		if (incount > lim_incount)
			return(1);
		else {
			if (outcount > lim_outcount) {
				if (nsucc > lim_first)
					return(1);
				else {
					incount = 0;
					outcount = 0;
				}
			}
		}
	}
	return(0);
}


try_temp()
{
	init();
	do {
		reconfigure(FORWARD);
		decide();
	} while (check_eq() == 0);
	printf("Finishing up temperature %ld\n", temp);
	update();
	if (!quench)
		while (score > score_lim) {
			reconfigure(FORWARD);
			decide();
		}
}

decide()
{
	long new_score, s;
	BOOLEAN acceptable = FALSE;

	if (caught) {
		report();
		caught = 0;
	}
	new_score = get_score();
	if (new_score < score) {
		if (new_score < best_score) {
			best_score = new_score;
			best_temp = temp;
			printf("Try %d Scored %ld at temperature %ld for:\n",
				scored, best_score, best_temp);
			show_setup(setup);
			copy(best, setup);	/* best <- setup */
		}
		acceptable = TRUE;
	} else if (temp > 0) {			/* uphill movement? */
		if (RAND(1000) < 1000*exp(((double) score-new_score)/temp)) {
			uphill++;
			acceptable = TRUE;
		}
	}

	if (acceptable) {
		s = ABS(score-new_score);
		if (s > max_change)
			max_change = s;
		score = new_score;
		s = new_score - scale;
		totscore += s;
		tot2score += s*s;
		nsucc++;
		if (ABS(avg_score - score) < half_sigma)
			incount++;
		else
			outcount++;

		if (score < bscore)
			bscore = score;
		else if (score > wscore)
				wscore = score;
	} else {
		reconfigure(BACK);
		nfail++;
	}
}

