comb(c, used, func)
struct config		*c;
int			*used;
int			(*func)();
{
	int		i, j, k, l;
	int		player;

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
					(*func)(c);
				}
			}
		}
	}
}
