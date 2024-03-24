#include <stdio.h>
#include <string.h>

#define USAGE			"Usage: %s\n"
char				*Pname;

#define PEOPLE			12
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

	tryhand(try, 0, func);
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

	for (i=0; i<PEOPLE+1; i++)
		try[i] = '\0';
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
