/*
 * solve
 *	for each game
 *		for each table
 *			deepest = 1
 *			best = select table(saved_config)
 *			record table(saved_config, best)
 *	make_report(saved_config)
 * 
 * select table(config)
 *	if (depth > deepest)
 *		if (times up)
 *			return none
 *		else
 *			deepest = depth
 *	for each possible table
 *		copy config to local_config
 *		record table(local_config, table)
 *		score config
 *	choose most likely choices
 *	for each choice
 *		copy config to local_config
 *		best, score = select player(local_config)
 *	return best, score
 */

#include <stdio.h>
#include <sys/signal.h>
#include "bridge.h"

struct config		global;
int			deepest;
int			depth;
long			time_limit;
long			nwork[MAX_DEPTH];
long			working[MAX_DEPTH];

long			time();


main(argc, argv)
int			argc;
char			*argv[];
{
	int		catch();

	signal(SIGALRM, catch);
	setup(&global);
	make_report(&global);
	alarm(REPORT_TIME);
	solve(&global);
	make_report(&global);
}


setup(c)
struct config		*c;
{
	int		i;

	/* First games'a gime */
	for (i = 0; i < PLAYERS; i++)
		c->player[i] = i;
	c->player[i++] = 'A'-'A';
	c->player[i++] = 'E'-'A';
	c->player[i++] = 'I'-'A';
	c->player[i++] = 'M'-'A';
	c->player[i++] = 'B'-'A';
	c->player[i++] = 'F'-'A';
	c->player[i++] = 'J'-'A';
	c->player[i++] = 'N'-'A';
	c->player[i++] = 'C'-'A';
	c->player[i++] = 'G'-'A';
	c->player[i++] = 'Q'-'A';
	c->player[i++] = 'U'-'A';
	c->player[i++] = 'D'-'A';
	c->player[i++] = 'H'-'A';
	c->player[i++] = 'R'-'A';
	c->player[i++] = 'V'-'A';
	c->player[i++] = 'K'-'A';
	c->player[i++] = 'O'-'A';
	c->player[i++] = 'S'-'A';
	c->player[i++] = 'W'-'A';
	c->player[i++] = 'L'-'A';
	c->player[i++] = 'P'-'A';
	c->player[i++] = 'T'-'A';
	c->player[i++] = 'X'-'A';
	c->player[i++] = 'A'-'A';
	c->player[i++] = 'F'-'A';
	c->player[i++] = 'K'-'A';
	c->player[i++] = 'P'-'A';
	c->player[i++] = 'B'-'A';
	c->player[i++] = 'E'-'A';
	c->player[i++] = 'L'-'A';
	c->player[i++] = 'O'-'A';
	c->player[i++] = 'C'-'A';
	c->player[i++] = 'H'-'A';
	c->player[i++] = 'S'-'A';
	c->player[i++] = 'X'-'A';
	c->player[i++] = 'D'-'A';
	c->player[i++] = 'G'-'A';
	c->player[i++] = 'T'-'A';
	c->player[i++] = 'W'-'A';
	c->player[i++] = 'I'-'A';
	c->player[i++] = 'N'-'A';
	c->player[i++] = 'Q'-'A';
	c->player[i++] = 'V'-'A';
	c->player[i++] = 'J'-'A';
	c->player[i++] = 'M'-'A';
	c->player[i++] = 'R'-'A';
	c->player[i++] = 'U'-'A';
	c->player[i++] = 'A'-'A';
	c->player[i++] = 'G'-'A';
	c->player[i++] = 'J'-'A';
	c->player[i++] = 'O'-'A';

	c->next_player = i;
}


make_report(c)
struct config		*c;
{
	int		player;
	char		table[PLAYERS * PLAYERS];

	for (player = 0; player < c->next_player; player++) {
		putchar(c->player[player] + 'A');
		if (POSITION(player) == PPERT-1)
			if (TABLE(player) == TABLES-1)
				putchar('\n');
			else
				putchar(' ');
	}

	printf("\nScore = %d\n", score_config(table,c));
	print_table(table);
	fflush(stdout);
}


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


score_config(t,c)
char			*t;
struct config		*c;
{
	int		i;
	int		score = 0;

	for (i = 0; i < PLAYERS * PLAYERS; i++)
		t[i] = 0;

	for (i = 0; i < c->next_player; i += PPERT) {
		score += TALLY(t,c->player[i],c->player[i+1]);
		score += TALLY(t,c->player[i],c->player[i+2]);
		score += TALLY(t,c->player[i],c->player[i+3]);
		score += TALLY(t,c->player[i+1],c->player[i+2]);
		score += TALLY(t,c->player[i+1],c->player[i+3]);
		score += TALLY(t,c->player[i+2],c->player[i+3]);
	}
	return(score);
}


score_table(t,c)
char			*t;
struct config		*c;
{
	register char	*p = &c->player[c->next_player];

	return (NOTALLY(t,p[0],p[1])
		+ NOTALLY(t,p[0],p[2])
		+ NOTALLY(t,p[0],p[3])
		+ NOTALLY(t,p[1],p[2])
		+ NOTALLY(t,p[1],p[3])
		+ NOTALLY(t,p[2],p[3]));
}


solve(c)
struct config		*c;
{
	int		score;
	int		i;

	for (i = c->next_player/PPERT; i < TABLES * GAMES; i++) {
		deepest = 0;
		depth = 0;
		time_limit = time((long *) 0) + SECONDS_PER_TABLE;
		score = best_table(c);
#ifdef DEBUG
printf("Chose %c%c%c%c for table %d, predicted score of %d in %d more tables\n",
	'A'+c->player[c->next_player],
	'A'+c->player[c->next_player+1],
	'A'+c->player[c->next_player+2],
	'A'+c->player[c->next_player+3], i, score, deepest);
#endif
		c->next_player += 4;
		make_report(c);
	}
}


best_table(c)
struct config		*c;
{
	char		used[PLAYERS];
	char		table[PLAYERS * PLAYERS];
	char		cream[PPERT * MAX_KEEP];
	int		cscore[MAX_KEEP];
	int		ncream = 0;
	int		tscore;
	int		score;
	int		best_score;
	int		best;
	int		player;
	int		i, j, k, l;
	int		m;

	/*
	 * If it's time to wrap up, or if we're at
	 * the end of the config, quit early.
	 */
	if ((player = c->next_player) == PLAYERS * GAMES)
		return(0);
	if (++depth > deepest)
#ifdef TIMEOUT
		if (time(0) > time_limit || depth > MAX_DEPTH) {
#else
		if (depth > MAX_DEPTH) {
#endif
			--depth;
			return(0);
		} else
			deepest = depth;
	
	/*
	 * Clear array
	 */
	for (i = 0; i < PLAYERS; i++)
		used[i] = 0;
	
	/*
	 * Score the current config once and just
	 * calculate the additional score later.
	 */
	score = score_config(table,c);
#ifndef DEBUG
if (MAX_DEPTH-depth > 2)
printf("At depth %d/%d, table %d, with score of %d",
	depth, deepest, c->next_player / PPERT, score);
#endif

	/*
	 * Mark the players that are already seated.
	 */
	for (i = THISGAME(player); i < player; i++)
		used[c->player[i]]++;

	/*
	 * Find out which table combinations are most promising.
	 */
	best_score = 99999;

	for (i = 0; i < PLAYERS-3; i++) {
		if (used[i])
			continue;
		c->player[player] = i;
		for (j = i+1; j < PLAYERS-2; j++) {
			if (used[j])
				continue;
			c->player[player+1] = j;
			for (k = j+1; k < PLAYERS-1; k++) {
				if (used[k])
					continue;
				c->player[player+2] = k;
				for (l = k+1; l < PLAYERS; l++) {
					if (used[l])
						continue;
					c->player[player+3] = l;

	/*
	 * A possible table has been configured.
	 * Evaluate it and record if appropriate.
	 * (This is likely to be where we spend
	 * most of our time.)
	 */
	tscore = score + score_table(table, c);
			
#ifndef DEBUG
printf("Checking table: %c%c%c%c scored %d\n",
	'A'+c->player[player], 'A'+c->player[player+1],
	'A'+c->player[player+2], 'A'+c->player[player+3], tscore);
#endif
	if (depth >= MAX_DEPTH && ncream > 0 && tscore >= cscore[0])
		continue;

	for (m = ncream; m > 0; m--)
		if (cscore[m-1] <= tscore)
			break;
	/*
	 * If the table scores in the top SOFT_MAX or if it
	 * scores 0 and there's less than MAX_KEEP then keep it.
	 */
	if ((tscore==0 && m<MAX_KEEP) || (m<SOFT_MAX && ncream<MAX_KEEP)) {
		if (tscore == 0 || ncream < SOFT_MAX)
			ncream++;
		insert(c,&cream[m*PPERT],ncream-m);
		insert_score(tscore,&cscore[m],ncream-m);
	}

				}
			}
		}
	}

#ifndef DEBUG
	if (MAX_DEPTH-depth > 2) {
		if (ncream > SOFT_MAX)
			printf(" kept %d\n", ncream);
		else
			printf("\n");
		fflush(stdout);
	}
#endif

	if (depth >= MAX_DEPTH) {
		/*
		 * Don't bother looking any deeper,
		 * just return the best.
		 */
		c->player[player+0] = cream[0];
		c->player[player+1] = cream[1];
		c->player[player+2] = cream[2];
		c->player[player+3] = cream[3];
		--depth;
		return(cscore[0]);
	}

	nwork[depth-1] = ncream;

	/*
	 * Investigate the top possibilities more fully.
	 */
	c->next_player = player + 4;
	for (i = 0; i < ncream; i++) {
		/*
		 * Since a score can not improve by looking deeper,
		 * don't bother looking if it's already too bad.
		 */
		if (best_score < cscore[i])
			continue;
		working[depth-1] = i;
		c->player[player+0] = cream[i * PPERT];
		c->player[player+1] = cream[i * PPERT + 1];
		c->player[player+2] = cream[i * PPERT + 2];
		c->player[player+3] = cream[i * PPERT + 3];
#ifndef DEBUG
printf("Investigating table: %c%c%c%c scored %d\n",
	'A'+c->player[player], 'A'+c->player[player+1],
	'A'+c->player[player+2], 'A'+c->player[player+3], cscore[i]);
#endif
		tscore = best_table(c);		/* recurse */
		if (tscore == 0)
			tscore = cream[i];
		if (tscore < best_score) {
			best_score = tscore;
			best = i;
			/*
			 * If we have perfection, then
			 * why look anymore.
			 */
			if (best_score == 0) {
				printf("Found perfection with %c%c%c%c at level %d\n",
				'A'+cream[i*PPERT], 'A'+cream[i*PPERT+1],
				'A'+cream[i*PPERT+2], 'A'+cream[i*PPERT+3], depth);
				break;
			}
		}
	}
	c->next_player = player;

	/*
	 * Return the best of the best.
	 */
	c->player[player+0] = cream[best * PPERT];
	c->player[player+1] = cream[best * PPERT + 1];
	c->player[player+2] = cream[best * PPERT + 2];
	c->player[player+3] = cream[best * PPERT + 3];
	--depth;
	return(best_score);
}


insert(c, cream, n)
struct config		*c;
char			*cream;
int			n;
{
	char		*place = cream;
	char		*player = &c->player[c->next_player];
	while (n--) {
		*(cream+4) = *(cream);
		*(cream+5) = *(cream+1);
		*(cream+6) = *(cream+2);
		*(cream+7) = *(cream+3);
		cream += 4;
	}
	*place++ = *player++;
	*place++ = *player++;
	*place++ = *player++;
	*place = *player;
}


insert_score(new,here,n)
int			new;
int			*here;
int			n;
{
	int		*place = here;

	while (n--) {
		*(here+1) = *here;
		here++;
	}
	*place = new;
}


catch()
{
	int		i;
	long		t;
	char		*buf, *ctime();
	char		*nl, *strchr();

	signal(SIGALRM, catch);
	time(&t);
	buf = ctime(&t);
	nl = strchr(buf, '\n');
	*nl = '\0';
	printf("%s  ", buf);
	for (i = 0; i < MAX_DEPTH-1; i++)
		printf("%d:%d/%d ", i+1, working[i], nwork[i]);
	printf("\n");
	fflush(stdout);
	alarm(REPORT_TIME);
}
