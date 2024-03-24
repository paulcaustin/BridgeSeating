/*
 * This structure holds an entire configuration of
 * all the players for all the games.  next_player
 * indicates how full the configuration is.
 */
#define TABLES		6
#define PPERT		4
#define GAMES		8
#define SECONDS_PER_TABLE	(60 * 15)
#define MAX_KEEP	4000
#define SOFT_MAX	25	/* Sort of the maximum non-zero's to keep */
#define MAX_DEPTH	8
#define REPORT_TIME	60*5

#define PLAYERS		(TABLES * PPERT)
#define POSITION(p)	((p) & 0x03)
#define TABLE(p)	((p) / PPERT % TABLES)

#define TALLY(t,p1,p2)	((t)[(p1)*PLAYERS+(p2)]++,(t)[(p2)*PLAYERS+(p1)]++)
#define NOTALLY(t,p1,p2)	((t)[(p1)*PLAYERS+(p2)])
#define THISGAME(p)	((p)-((p) % PLAYERS))
#define THISTABLE(p)	((p)-((p) % PPERT))

struct config {
	char	player[PLAYERS * GAMES];
	int	next_player;
};
