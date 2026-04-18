#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 1))
	{
		fprintf(stderr, "table_create returned 0\n");
		return (1);
	}
	if (table.count != 1)
	{
		fprintf(stderr, "expected count 1, got %d\n", table.count);
		return (1);
	}
	if (table.philosophers[0].fork_left != &table.forks[0])
	{
		fprintf(stderr, "fork_left should be forks[0]\n");
		return (1);
	}
	if (table.philosophers[0].fork_right != &table.forks[0])
	{
		fprintf(stderr, "fork_right should be forks[0] for single philosopher\n");
		return (1);
	}
	table_free(&table);
	return (0);
}
