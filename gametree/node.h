#define PPERT		4

struct node {
	struct node	*sibling;
	struct node	*child;
	char		p[PPERT];
	int		score;
};
typedef struct node	node_t;
