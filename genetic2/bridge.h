#define TABLES		(players / PPERT)	/* # of tables per game */
#define PPERT		4	/* # of people per table, don't change */
#define GAMES		60	/* maximum # of games (or hands) */
#define MAXPOP		500	/* Absolute max allowable population */
#define MUTATIONS	50	/* maximum tries to create a successful child */
#define DEF_SORT	2	/* Chromosone sort style */
#define REPORT_FREQ	100000
#define FANCY_COMPARE	1
#define DISASTER_SAVE	10

#define PLAYERS		50	/* maximum players per game */
#define SEATS		(PLAYERS * GAMES)	/* total storage size */
#define POSITION(p)	((p) % PPERT)		/* position within table */
#define TABLE(p)	((p) / PPERT % TABLES)	/* table within game */

#define BAD_PARTNER	5	/* score for bad partners */
#define BAD_TABLE	1	/* score for bad tables */
#define BAD_WITH	3	/* score for bad tables */

/*
 * This totally disgusting macro increments the tallies
 * within my scoring table.  "p2" must never be more than
 * "players".
 * 
 * Use TALLY2 to symmetrically increment two things.  The "t"
 * table must be at least players * players big to use TALLY2.
 */
#define TALLY(t,p1,p2)	((t)[(p1)*players+(p2)]++)
#define TALLY2(t,p1,p2)	((t)[(p1)*players+(p2)]++,(t)[(p2)*players+(p1)]++)

/*
 * Look up a tally but don't change it -- not used.
 */
#define NOTALLY(t,p1,p2)	((t)[(p1)*players+(p2)])

/*
 * The Data structure that contains a chromosome.
 */
typedef struct _chrom {
	int		score;
	int		old_score;
	unsigned char	p[SEATS];
} CHROM;

/*
 * Standard C Library random numbers suck.
 * Most C Libraries have these improved ones.
 */
#define srand		srand48
#define rand		lrand48

/*
 * Features
 */
#define SCORE_TABLE1
#define SCORE_TABLE2
#define SCORE_TABLE3
#undef SCORE_GENDER	/* Odd players are male, even are female */
#define SCORE_PLAYWITH
#define SCORE_TABLING
#define SCORE_TABLESTAY
#define SHOW_SEATING
#define SHOW_TABLING
#define SCORE_PARTNERS

#undef SCORE_SOMETHING	/* ?? Golf? Playing early vs late? */
#undef SHOW_EXTRA	/* ?? debug I think */
#undef OLDSIGSTUFF
#undef SHOW_PICKCOUNTS
#define TRY_MUTATE_HARDER
#undef USE_OPTIMIZE

/*
 * Defaults
 */
#define DEF_MAXITERATE	100000000
#define DEF_TERMINATE	1000000
#define DEF_NPOP	100
#define DEF_MAXSWAP	40
#define DEF_PLAYERS	24
#define DEF_GAMES	8
#define DEF_DISASTER	2000000
#undef DEBUG_SCORE

#undef QUICKTEST
#ifdef QUICKTEST
#undef DEF_TERMINATE
#define DEF_TERMINATE	10000
#endif

