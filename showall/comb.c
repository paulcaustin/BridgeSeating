comb(old, start, num, max, func)
char			*old;
int			start;
int			num;
int			max;
int			(*func)();
{
	char		new[max];
	int		i;

	memcpy(new, old, start);

	for (i=start+1; i < max-num; i++) {
		new[start] = old[i];
		if (num == 1)
			(*func)(new);
		else
			comb(new, start+1, num-1, max, func);
	}
}
