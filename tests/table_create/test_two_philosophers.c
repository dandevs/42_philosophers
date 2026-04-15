#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 2))
	{
		printf("table_create returned 0\n");
		return (1);
	}
	if (table.philosophers[0].fork_left != &table.forks[0]
		|| table.philosophers[0].fork_right != &table.forks[1])
	{
		printf("philo[0] fork assignment wrong\n");
		return (1);
	}
	if (table.philosophers[1].fork_left != &table.forks[1]
		|| table.philosophers[1].fork_right != &table.forks[0])
	{
		printf("philo[1] fork assignment wrong\n");
		return (1);
	}
	table_destroy(&table);
	return (0);
}
