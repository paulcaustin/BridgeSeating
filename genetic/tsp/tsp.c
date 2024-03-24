#include <stdio.h>
#include <math.h>
#include <time.h>
double strtod();

#define MAXCITY		104
#define MAXPOP		500
#define NAMESIZE	24

float			dist[MAXCITY][MAXCITY] = { 0 };
unsigned char		edges[MAXCITY][6] = { 0 };
unsigned char		edgecitys = 0;
unsigned char		candcity[MAXCITY] = { 0 };

typedef struct _chrom {
	float		score;
	unsigned char	tour[MAXCITY];
} CHROM;

CHROM			*popptr[MAXPOP] = { 0 };
CHROM			pop[MAXPOP] = { 0 };
CHROM			offspring = 0;

typedef struct _city {
	float		xpos, ypos;
	char		cityname[NAMESIZE];
} CITY;

int			ncities = 0;
CITY			cities[MAXCITY] = { 0 };
int			verbose = 0;
long			mutatecnt = 0;
long			crosscnt = 0;
long			insertcnt = 0;
long			pickcnt[MAXPOP] = { 0 };

#define srand		srand48
#define rand		lrand48

void printedges()
{
	int		chx;

	printf("Edge List:\n");
	for (chx = 0; chx < ncities; chx++)
		printf(" City = %3d N=%1d %3d %3d %3d %3d %d\n",
		  chx, edges[chx][0], edges[chx][1], edges[chx][2],
		  edges[chx][3], edges[chx][4], edges[chx][5]);
	return;
}


void printtour(chrom, o, title, mode)
CHROM			*chrom;
FILE			*o;
char			*title;
int			mode;
{
	int		cx;
	CITY		*city;

	printf(" %-10s %12.3f", title, chrom->score);
	for (cx = 0; cx < ncities; cx++) {
		if (mode) {
			city = &cities[chrom->tour[cx]];
			printf("\n  %3d %12.3f %12.3f %s",
			  cx+1, city->xpos, city->ypos, city->cityname);
		} else
			printf("%3d", chrom->tour[cx]);
	}
	printf("\n");
	return;
}


void initdist()
{
	int		cx;
	int		i, j;
	double		w, x;
	double		d;

	if (verbose) printf("Initializing distances...");
	for (i = 0; i < ncities; i++) {
		dist[i][i] = 0;
		for (j = 0; j < i; j++) {
			w = cities[i].xpos - cities[j].xpos;
			x = cities[i].ypos - cities[j].ypos;
			d = sqrt(w * w + x * x);
			dist[i][j] = d;
			dist[j][i] = d;
		}
	}
	if (verbose) printf("Done\n");

	return;
}


/*
 * readcities -- read in city information
 * Format of cities file:
 *	!comment line
 *	xpos\ty-pos\tcity_name (no blanks)
 */
int readcities(fn)
char			*fn;
{
	int		cx;
	FILE		*in;
	int		i;
	char		*s;
	char		*a;
	char		buf[256];

	if ((in = fopen(fn,"r")) == NULL) {
		perror(fn);
		exit(1);
	}

	cx = 0;
	memset((char *) &cities[0], 0, sizeof(cities));
	for (; cx < MAXCITY;) {
		if ((s = fgets(buf, sizeof(buf), in)) == NULL)
			break;
		while (*s && *s <= ' ')
			s++;
		if (*s == '\0' || *s == '!')
			continue;
		cities[cx].xpos = strtod(s, &a);
		if (*a > ' ') {
			printf("Illegal X-Position in line: %s", buf);
			continue;
		}
		s = a;
		cities[cx].ypos = strtod(s, &a);
		if (*a > ' ') {
			printf("Illegal Y-Position in line: %s", buf);
			continue;
		}
		s = a;
		while (*s && *s <= ' ')
			s++;
		i = NAMESIZE;
		for (a = &cities[cx].cityname[0]; *s > ' '; s++)
			if (--i >= 0)
				*a++ = *s;
		*a = 0;
		if (verbose)
			printf("Reading City %d x=%.3f y=%.3f\t%s\n",
			  cx, cities[cx].xpos, cities[cx].ypos,
			  cities[cx].cityname);
		cx++;
	}
	ncities = cx;
	fclose(in);

	if (ncities < 5) {
		printf("A minimum of five cities are required\n");
		exit(1);
	}

	initdist();
	return(0);
}

double evaluate(chrom)
CHROM			*chrom;
{
	unsigned char	ccity;
	unsigned char	pcity;
	int		cx;
	double		w = 0.0;

	for (cx = 0; cx < ncities; cx++) {
		ccity = chrom->tour[cx];
		pcity = chrom->tour[(cx == 0 ? ncities : cx) - 1];
		w += dist[ccity][pcity];
	}
	chrom->score = w;
	return(w);
}

int localopt(chrom)
CHROM			*chrom;
{
	int		swaped = 0;
	int		p0, p1, p2, p3;
	int		cx;
	int		ec;
	unsigned char	*u;
	double		c, n;

	u = &chrom->tour[0];
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
}


void decompose(chrom)
CHROM			*chrom;
{
	unsigned char	ccity;
	unsigned char	pcity;
	int		cx;
	int		ci;
	unsigned char	*u;

	for (cx = 0; cx < ncities; cx++) {
		ccity = chrom->tour[cx];
		pcity = chrom->tour[(cx == 0 ? ncities : cx) - 1];
		u = &edges[ccity][0];
		for (ci = *u; ci > 0; --ci)
			if (pcity == u[ci])
				goto foundpcity;
		ci = ++u[0];
		u[ci] = pcity;
foundpcity:
		u = &edges[pcity][0];
		for (ci = *u; ci > 0; --ci)
			if (ccity == u[ci])
				goto foundccity;
		ci = ++u[0];
		u[ci] = ccity;
foundccity:
		;
	}
	return;
}


void removecity(ccity)
int			ccity;
{
	int		cx;
	int		ci;
	int		wl;
	unsigned char	*u;
	unsigned char	*du;

	edges[ccity][5] = 1;
	for (cx = 0; cx < ncities; cx++) {
		u = &edges[cx][0];
		ci = u[0];
		du = &u[ci];
		for (; ci > 0; ci--, du--) {
			if (*du == ccity) {
				for (wl = *u - ci; --wl >= 0; du++)
					du[0] = du[1];
				u[0]--;
				break;
			}
		}
	}
	return;
}


int findfewest(list)
int			list;
{
	int		cx;
	int		ccity;
	int		nedge;
	unsigned char	*u;
	unsigned char	*e;

	edgecitys = 5;
	candcity[0] = 0;
	u = &edges[list][0];
	e = &candcity[1];

	for (cx = *u; cx > 0; cx--) {
		ccity = u[cx];
		if (edges[ccity][5] == 0) {
			nedge = edges[ccity][0];
			if (nedge < edgecitys) {
				edgecitys = nedge;
				e = &candcity[1];
				*e++ = ccity;
			} else if (nedge == edgecitys) {
				*e++ = ccity;
			}
		}
	}
	candcity[0] = e - &candcity[1];

	if (candcity[0] == 0) {
		e = &candcity[1];
		for (ccity = 0; ccity < ncities; ccity++) {
			if (ccity != list && edges[ccity][5] == 0)
				*e++ = ccity;
		}
		candcity[0] = e - &candcity[1];
	}

	if (candcity[0] <= 1) {
		ccity = candcity[1];
	} else {
		ccity = candcity[((int)(rand() % candcity[0])) + 1];
	}

	return(ccity);
}


double crossover(child, par1, par2, opt)
CHROM			*child;
CHROM			*par1, *par2;
int			opt;
{
	int		chx;
	int		ccity;
	int		p1c, p2c;

	memset((char *) &edges[0][0], 0, sizeof(edges));

	decompose(par1);
	decompose(par2);

	if (verbose)
		printedges();
	
	p1c = par1->tour[0];
	p2c = par2->tour[0];
	if (edges[p1c] == edges[p2c])
		ccity = (rand() % 1000) >= 500 ? p1c : p2c;
	else if (edges[p1c] < edges[p2c])
		ccity = p1c;
	else
		ccity = p2c;
	
	chx = 0;
	for(;;) {
		child->tour[chx++] = ccity;
		if (chx >= ncities)
			break;
		removecity(ccity);
		ccity = findfewest(ccity);
	}

	if (opt)
		localopt(child);
	
	evaluate(child);

	if (verbose)
		printtour(child, stdout, "Cross-Over", 0);

	crosscnt++;
	return(child->score);
}


mutate(chrom, iter, opt)
CHROM			*chrom;
int			iter;
int			opt;
{
	unsigned char	*up, u;
	int		a, b;

	up = &chrom->tour[0];
	while (iter > 0) {
		a = rand() % ncities;
		b = rand() % ncities;
		if (a != b) {
			u = up[a];
			up[a] = up[b];
			up[b] = u;
			iter--;
		}
	}

	if (opt)
		localopt(chrom);

	evaluate(chrom);

	if (verbose)
		printtour(chrom, stdout, "Mutated", 0);

	mutatecnt++;
	return;
}


int initcmp(pa, pb)
char			*pa, *pb;
{
	double		w;
	CHROM		*pca, *pcb;

	pca = *((CHROM **) pa);
	pcb = *((CHROM **) pb);
	w = pca->score - pcb->score;
	if (w > 0.0)
		return(1);
	else if (w < 0.0)
		return(-1);
	else
		return(0);
}


void initpopulation(npop)
int			npop;
{
	int		cx;
	CHROM		*chrom;
	int		gx;

	if (verbose)
		printf("Initializing population...\n");
	memset((char *) &pop[0], 0, sizeof(pop));
	for (cx = 0; cx < npop; cx++) {
		chrom = &pop[cx];
		popptr[cx] = chrom;
		for (gx = 0; gx < ncities; gx++)
			chrom->tour[gx] = gx;
		mutate(chrom, ncities*2, 1);
	}
	if (verbose)
		printf("Sorting population...\n");
	qsort((void *) &popptr[0], npop, sizeof(popptr[0]),initcmp);
	if (verbose)
		printf("Done.\n");
	return;
}


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
	double		w;
	CHROM		*chrom;

	for (;;) {
		p1 = pickparent(npop);
		p2 = pickparent(npop);
		if (p1 != p2)
			break;
	}

	if (verbose) {
		printf("Evolve parents %d and %d\n", p1, p2);
		printtour(popptr[p1], stdout, "Parent 1", 0);
		printtour(popptr[p2], stdout, "Parent 2", 0);
	}

	duplicate = 1;
	size = sizeof(float) + ncities;
	for (mutflag = 0; duplicate != 0 && mutflag < 10; mutflag++) {
		if (mutflag) {
			swap = (rand() % maxswap) + 1;
			mutate(&offspring, swap, opt);
		} else
			crossover(&offspring, popptr[p1], popptr[p2], opt);
		duplicate = 0;
		for (px = 0; px < npop; px++) {
			w = (&offspring)->score - popptr[px]->score;
			if (w < 0.0)
				break;
			if (w == 0 && 
			  memcmp((char *) &offspring, (char *) popptr[px], size) == 0) {
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


main(ac, av)
int			ac;
char			*av[];
{
	char		*fn;
	int		npop;
	int		maxswap;
	long		maxiter;
	long		iter;
	long		termcount;
	long		bestctr;
	long		t0, t1, t2;
	int		w;
	int		opt;
	double		best;
	static char	*help[] = {
"Use: gatsp CityFile Population MaxIterations TermCount MaxSwapCnt OptFlag\n",
"Ex: gatsp city100.txt 50 100000 10000 5 1 > res.fil\n",
"    use <city100.txt> as the problem file\n",
"    50 possible solutions in the population\n",
"    Iterate a maximum of 100,000 times.\n",
"    If a better solution is not found in 10,000 iterations, stop\n",
"    When mutating, swap up to five pairs of cities\n",
"    Perform local optimization\n",
"    Write the results to the file <res.file>\n",
0 };

	time(&t0);
	if (ac !=7 && ac != 8) {
		for (w = 0; help[w] != NULL; w++)
			printf(help[w]);
		exit(0);
	}

	fn = av[1];
	npop = atol(av[2]);
	maxiter = atol(av[3]);
	termcount = atol(av[4]);
	maxswap = atol(av[5]);
	opt = atol(av[6]);
	verbose = (ac == 8) ? atol(av[7]) : 0;

	if (npop > MAXPOP)
		npop = MAXPOP;
	
	if (readcities(fn) < 0)
		exit(1);
	
	srand(t0);
	initpopulation(npop);
	best = popptr[0]->score;
	bestctr = 0;

	iter = 0;
	if (verbose || 1) {
		printf("Input file = %s\tCities = %d\n", fn, ncities);
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
		if (verbose == 0 && (iter % 100) == 0) {
			printf("%9ld %9ld %6.3f %9ld %9ld %9ld\n",
			  iter, bestctr, popptr[0]->score, mutatecnt,
			  crosscnt, insertcnt);
			mutatecnt = crosscnt = insertcnt = 0;
		}
		iter++;
	}
	time(&t2);

	printf("\n%9ld %9ld %12.3f %9ld %9ld %9ld  *\n\n",
	  iter, bestctr, popptr[0]->score,
	  mutatecnt, crosscnt, insertcnt);
	
	printf("\nTotal Time = %ld seconds; Search Time = %ld seconds.\n",
	  t2 - t0, t2 - t1);
	if (iter >= maxiter)
		printf("Terminated due to Maximum Iteration Count\n");
	if (bestctr >= termcount)
		printf("Terminated due to Maximum Patience Reached\n");
	printtour(popptr[0], stdout, "Best Tour", 1);

	printf("\nParent Pick Counts\nRank\tCount\n");
	for (w = 0; w < npop; w++)
		printf("%4d\t%9ld\n", w, pickcnt[w]);
	return(0);
}
