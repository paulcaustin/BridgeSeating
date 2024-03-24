#define TABLES		6	/* # of tables per game */
#define PPERT		4	/* # of people per table */
#define GAMES		8	/* # of games (or hands) */
#define MAXPOP		250	/* Absolute max allowable population */
#define MUTATIONS	50	/* maximum to make child unique */

#define PLAYERS		(TABLES * PPERT)	/* players per game */
#define SEATS		(PLAYERS * GAMES)	/* total storage size */
#define POSITION(p)	((p) & 0x03)		/* position within table */
#define TABLE(p)	((p) / PPERT % TABLES)	/* table within game */

/*
 * This totally disgusting macro increments the tallies
 * within my scoring table.  Due to symmetry there are
 * two things incremented.
 */
#define TALLY(t,p1,p2)	((t)[(p1)*PLAYERS+(p2)]++,(t)[(p2)*PLAYERS+(p1)]++)

#ifdef NOTUSED			/* These macros not used in this program */
/*
 * Look up a tally but don't change it -- not used.
 */
#define NOTALLY(t,p1,p2)	((t)[(p1)*PLAYERS+(p2)])
/*
 * Data position of first seat of this game.
 */
#define THISGAME(p)	((p)-((p) % PLAYERS))
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
	unsigned char	p[SEATS];
} CHROM;

/*
 * Standard C Library random numbers suck.
 * Most C Libraries have these improved ones.
 */
#define srand		srand48
#define rand		lrand48
