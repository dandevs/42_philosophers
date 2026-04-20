#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 1))
		return (fprintf(stderr, "table_create returned 0\n"), 1);
	if (table.count != 1)
		return (fprintf(stderr, "expected count 1, got %d\n", table.count), 1);
	if (table.philosophers[0].fork_left != &table.forks[0])
		return (fprintf(stderr, "fork_left should be forks[0]\n"), 1);
	if (table.philosophers[0].fork_right != &table.forks[0])
		return (fprintf(stderr, "fork_right should be forks[0] for single philosopher\n"), 1);
	table_free(&table);
	return (0);
}
