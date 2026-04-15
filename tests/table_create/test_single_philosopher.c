#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 1))
	{
		printf("table_create returned 0\n");
		return (1);
	}
	if (table.count != 1)
	{
		printf("expected count 1, got %d\n", table.count);
		return (1);
	}
	if (table.philosophers[0].fork_left != &table.forks[0])
	{
		printf("fork_left should be forks[0]\n");
		return (1);
	}
	if (table.philosophers[0].fork_right != &table.forks[0])
	{
		printf("fork_right should be forks[0] for single philosopher\n");
		return (1);
	}
	table_destroy(&table);
	return (0);
}
