#define TABLES		3	/* # of tables per game */
#define PPERT		4	/* # of people per table, don't change */
#define GAMES		14	/* maximum # of games (or hands) */
#define MAXPOP		500	/* Absolute max allowable population */
#define MUTATIONS	4	/* maximum tries to create a successful child */

#define PLAYERS		12	/* maximum players per game, see -p */
#define PRESENT		9	/* players actually present, see score() */
#define SEATS		(PLAYERS * GAMES)	/* total storage size */
#define POSITION(p)	((p) % PPERT)		/* position within table */
#define TABLE(p)	((p) / PPERT % TABLES)	/* table within game */

#define BAD_PARTNER	50	/* score for bad partners */
#define BAD_TABLE	20	/* score for bad tables */

/*
 * This totally disgusting macro increments the tallies
 * within my scoring table.  Due to symmetry there are
 * two things incremented.
 */
#define TALLY(t,p1,p2)	((t)[(p1)*players+(p2)]++,(t)[(p2)*players+(p1)]++)

#ifdef NOTUSED			/* These macros not used in this program */
/*
 * Look up a tally but don't change it -- not used.
 */
#define NOTALLY(t,p1,p2)	((t)[(p1)*players+(p2)])
/*
 * Data position of first seat of this game.
 */
#define THISGAME(p)	((p)-((p) % players))
/*
 * Data position of first seat of this table.
 */
#define THISTABLE(p)	((p)-((p) % PPERT))
#endif

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
