#include <stdio.h>
#include "node.h"

static node_t		*tree_head = NULL;
static node_t		*free_head = NULL;

/*
 * Remove a node from the tree (put on free list)
 *	n	address of the pointer to the node to be deleted.
 */
delete_node(n)
node_t			**n;
{
	node_t		*tmp;

	tmp = *n;
	if (tmp->child != NULL) {
		fprintf(stderr,"delete_node can't delete a parent\n");
		sleep(10);
		exit(1);
	}
	*n = tmp->sibling;
	tmp->sibling = free_head;
	free_head = tmp;
}


/*
 * Remove a string of nodes from the tree (put on free list)
 *	n	address of the pointer to the first node to be deleted.
 */
delete_string(n)
node_t			**n;
{
	while (*n)
		delete_node(n);
}


/*
 * Place a node in a tree.
 *	place	address of the pointer to be pointed to n
 *	n	address of the node to be added
 */
add_node(place,n)
node_t			**place;
node_t			*n;
{
	n->sibling = *place;
	*place = n;
}


node_t *
new_node()
{
	int		i = 10;
	node_t		*n;

	if (free_head == NULL) {
		n = (node_t *) malloc(i * sizeof(node_t));
		if (n == NULL) {
			fprintf(stderr,"new_node: out of memory\n");
			exit(1);
		}
		for (; i > 0; i--, n++) {
			n->sibling = free_head;
			free_head = n;
		}
	}

	n = free_head;
	free_head = n->sibling;
	n->sibling = NULL;
	n->child = NULL;
	return(n);
}


dump_tree(fp, n)
FILE			*fp;
node_t			*n;
{
	node_t		*tmp;

	for (tmp = n; tmp != NULL; tmp = tmp->sibling) {
		fprintf(fp, "=% 4s\t%d\n", tmp->p, tmp->score);
		dump_tree(fp, tmp->child);
	}
	fprintf(fp, "NULL\n");
}


static char		buf[BUFSIZ];

read_tree(fp, n)
FILE			*fp;
node_t			**n;
{
	node_t		*tmp;

	while (fgets(buf, BUFSIZ-1, fp) != NULL) {
		if (strcmp(buf, "NULL\n") == 0)
			return;
		tmp = new_node();
		if (sscanf(buf, "=%c%c%c%c %d", &tmp->p[0], &tmp->p[1],
		    &tmp->p[2], &tmp->p[3], &tmp->score) != 5) {
			fprintf(stderr, "read_tree: format error\n");
			fprintf(stderr, "input line: %s", buf);
			exit(1);
		}
		add_node(n, tmp);
		read_tree(fp, &(*n)->child);
		n = &tmp->sibling;
	}
}

#ifdef TEST
main()
{
	read_tree(stdin, &tree_head);
	dump_tree(stdout, tree_head);
}
#endif
