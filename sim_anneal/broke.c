/*
 * Determine good seating arrangement for a bridge tournament
 *
 * Don't sit at the same table more than twice
 * Don't partner with anyone more than once
 * Don't play against anyone more than twice
 */
#include <stdio.h>

#define TABLES		4
#define PLAYERS		16
#define GAMES		8
#define PPERT		(PLAYERS / TABLES)

#define PART(nn)	((nn) ^ 2)		/* guy accross the table */
#define OPP1(nn)	((nn) ^ 1)		/* guy to one side */
#define OPP2(nn)	((nn) ^ 3)		/* guy to the other side */
#define TABLE(nn)	((nn) >> 2)		/* which table */
#define PERS(xx,yy)	(((xx)<<2)|(yy))

int		record[GAMES][PLAYERS];
int		table[PLAYERS][TABLES];
int		part[PLAYERS][PLAYERS];
int		with[PLAYERS][PLAYERS];
int		a[PLAYERS];
int		used[PLAYERS];
int		game = 0;
long		cnt = 0;
int		bad_table;

main(argc,argv)
int argc;
char *argv[];
{
	int		hand[PLAYERS];
	int		i;
	long		time();

	if (argc > 1) {
/*		restart(a); */
		printf("Say what?\n");
		exit(1);
	} else
		first_hand(a);

	while (game >= 0) {
		if ((++cnt & 0x1fff) == 0) {	/* show every 8192 */
			printf("Try #%ld:", cnt);
			show_hand(a);
		}
		if (valid_hand(a)) {		/* also sets bad_table */
			record_hand(a);		/* also increments game */
			if (game == GAMES) {
				gotit();
				exit(0);
			}
			found_one(a);
		}
		while (next_hand(bad_table))	/* until no error, back up */
			if (--game) {		/* silly to change game 0 */
				init_record();
				for (i=0; i<game; i++)
					stat_hand(&record[i][0]);
				for (i=0; i<PLAYERS; i++)
					a[i] = record[game][i];
				bad_table = 0;
				all_used();
			} else {
				printf("All possibilities exhausted\n");
				exit(1);
			}
		}
}


/*
 * Don't sit at the same table more than twice
 * Don't partner with anyone more than once
 * Don't play against anyone more than twice
 */
valid_hand(a)
int		a[];
{
	register int	i;		/* counter */
	register int	p;		/* player */

	for (i=PLAYERS-1; i>=0; i--) {
		p = a[i];
#ifdef REMOVED
		if (table[p][TABLE(i)] == 2) {
			bad_table = TABLE(i);
			return(0);		/* this table too many times */
		}
#endif
		if (part[p][a[PART(i)]]) {
			bad_table = TABLE(i);
			return(0);		/* partnered too many times */
		}
		if (with[p][a[PART(i)]] == 2) {	/* check partner */
			bad_table = TABLE(i);
			return(0);		/* with too many times */
		}
		if (with[p][a[OPP1(i)]] == 2) {	/* check one opponent */
			bad_table = TABLE(i);
			return(0);		/* with too many times */
		}
		if (with[p][a[OPP2(i)]] == 2) {	/* check other opponent */
			bad_table = TABLE(i);
			return(0);		/* with too many times */
		}
	}
	bad_table = TABLES-1;			/* if valid, every table */
	return(1);				/* must change for next */
}


record_hand(a)
int		a[];
{
	register int	i;	/* counter */
	int		p;	/* player */

	for (i=0; i<PLAYERS; i++) {
		p = record[game][i] = a[i];
		table[p][TABLE(i)]++;		/* at this table */
		part[p][a[PART(i)]]++;		/* with this partner */
		with[p][a[PART(i)]]++;		/* with these other players */
		with[p][a[OPP1(i)]]++;
		with[p][a[OPP2(i)]]++;
	}
	game++;
}


stat_hand(a)
int		a[];
{
	register int	i;	/* counter */
	int		p;	/* player */

	for (i=0; i<PLAYERS; i++) {
		p = a[i];
		table[p][TABLE(i)]++;		/* at this table */
		part[p][a[PART(i)]]++;		/* with this partner */
		with[p][a[PART(i)]]++;		/* with these other players */
		with[p][a[OPP1(i)]]++;
		with[p][a[OPP2(i)]]++;
	}
}


init_record()
{
	int		i, j;

	for (i=0; i<PLAYERS; i++) {
		for (j=0; j<TABLES; j++)
			table[i][j] = 0;
		for (j=0; j<PLAYERS; j++) {
			part[i][j] = 0;
			with[i][j] = 0;
		}
	}
}


next_hand(table)
int	table;
{
	for (; table < TABLES; table++)
		if (!fill_tables(table))	/* fill tables to the right */
			break;

	if (table < TABLES)
		return(0);

	return(1);
}


/* try filling table, and tables to the right */
fill_tables(table)
int		table;
{
	register int		i;

again:
	for (i = table-1; i >=0; i--)
		discard(i);
	if (change_table(table))	/* try changing 1,2,3 players */
		return(1);
	for (i = table-1; i >= 0; i--)
		if (fill1_table(i))		/* fill other tables */
			goto again;
	return(0);
}


fill1_table(tbl)
int		tbl;
{
	register int		i;
	register int		player = 0;

	for (i = PPERT-1; i >= 0; i--)
		for (; player < PLAYERS; player++)
			if ((!used[player]) && table[player][tbl] < 2) {
				used[a[PERS(tbl,i)] = player]++;
				break;
			}

	if (player < PLAYERS)
		return(0);

	return(1);
}


change_table(table)
int		table;
{
	register int		i;

#ifdef DEBUG
	printf("change table %d\n",table);
#endif
	for (i=0; i<PPERT; i++)
		if (!new_guys(table,i))
			break;
	if (i == PPERT)
		return(-1);
	return(0);
}


new_guys(tbl,seat)
int		tbl;
int		seat;
{
	register int		i;
	register int		player;

#ifdef DEBUG
	printf("new guys %d, %d\n",tbl,seat);
#endif
again:
	player = ++a[PERS(tbl,seat)];
	if (player >= PLAYERS)
		return(1);

	for (i=seat; i>=0; i--)
		used[a[PERS(tbl,i)]] = 0;
	
	for (i=seat; i >= 0; i--) {
#ifdef DEBUG
		printf("new guys filling seat %d, with %d\n",i, player);
#endif
		for (; player < PLAYERS; player++)
			if ((!used[player]) && table[player][tbl] < 2) {
				used[a[PERS(tbl,i)] = player]++;
				break;
			}
	}

	if (player < PLAYERS)
		return(0);

	goto again;
}


discard(table)
int		table;
{
	register int		i;

/*	printf("Discard %d\n",table); */
	for (i=0; i<PPERT; i++)
		used[a[PERS(table,i)]] = 0;
}


first_hand()
{
	int		i;

	for (i=0; i<PLAYERS; i++) {
		a[i] = PLAYERS-i-1;
		used[i] = 1;
	}
	game = 0;
}


show_hand(a)
int a[];
{
	register int		i;

	for (i=0; i<PLAYERS; i++) {
		if ((i & 3) == 0)	/* quick for i % PPERT == 0 */
			putchar('\t');
		printf(" %d",a[i]);
	}
	putchar('\n');
}


found_one(a)
int a[];
{
	printf("Got game #%d:", game);
	show_hand(a);
}


gotit()
{
	int		g, p;

	printf("\n\n*************** Solution *****************\n");
	for (g=0; g<GAMES; g++) {
		printf("Game #%d:", g);
		show_hand(&record[g][0]);
	}
}


all_used()
{
	register int		i;

	for (i=0; i<PLAYERS; i++)
		used[i] = 1;
}
