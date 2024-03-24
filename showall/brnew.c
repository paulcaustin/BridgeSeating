#include <stdio.h>
#include <string.h>

#define USAGE			"Usage: %s\n"
char				*Pname;

#define PEOPLE			16
#define ROUNDS			8
#define FALSE			0
#define TRUE			1
int				show_hand();

main(argc,argv)
int				argc;
char				*argv[];
{
	int			ch;
	extern int		optind;
	extern char		*optarg;

	if ((Pname = strrchr(argv[0], '/')) == NULL)
		Pname = argv[0];
	else
		Pname++;

	while ((ch = getopt(argc,argv,"")) != EOF)
		switch (ch) {
		default:
			usage();
		}
	
	trysets(show_hand);
}


trysets(func)
int	(*func)();
{
	static char	solution[ROUNDS][PEOPLE+1];
	static char	try[PEOPLE+1];

#ifdef LATER
	for (i=0; i<ROUNDS; i++)
		init_try(solution[i]);
#endif
	
	init_try(try);

	comb(try, 0, 0, 3, func);
}


comb(old, pos, start, num, func)
char			*old;
int			pos;
int			start;
int			num;
int			(*func)();
{
	char		new[PEOPLE+1];
	char		ch;
	int		i;

#ifdef DEBUG
	printf("comb: pos=%d, start=%d, num=%d, old=%s\n", pos, start, num, old);
#endif
	memcpy(new, old, PEOPLE+1);

	for (i=start; i < PEOPLE-num; i++) {
		ch = new[i];
		new[i] = new[pos];
		new[pos] = ch;
		if (num == 0)
			if (pos+5 == PEOPLE)
				(*func)(new);
			else	/* next table */
				comb(new, pos+1, pos+1, 3, func);
		else		/* rest of table */
			comb(new, pos+1, i+1, num-1, func);
	}
}


tryhand(try, start, func)
char			try[];
int			start;
int			(*func)();
{
	int		i;
	int		last = 'a' + PEOPLE - 3 + (start & 0x03);
	register int	val;

	if ((start & 0x03) == 0)
		val = 'a';
	else
		val = try[start-1];

	for (; val < last; val++) {
		try[start] = '\0';		/* terminate for strchr */
		if (strchr(try, val) == NULL) {
			try[start] = val;
			if (start == PEOPLE-1)
				(*func)(try);
			else {
				tryhand(try, start+1, func);
			}
		}
	}
}


init_try(try)
char			try[];
{
	int		i;

	for (i=0; i<PEOPLE; i++)
		try[i] = 'a'+i;

	try[PEOPLE] = '\0';
}


show_hand(try)
char			try[];
{
	puts(try);
}


usage()
{
	fprintf(stderr,USAGE,Pname);
	exit(1);
}
