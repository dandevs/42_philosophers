#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 2))
		return (fprintf(stderr, "table_create returned 0\n"), 1);
	if (table.philosophers[0].fork_left != &table.forks[0]
		|| table.philosophers[0].fork_right != &table.forks[1])
		return (fprintf(stderr, "philo[0] fork assignment wrong\n"), 1);
	if (table.philosophers[1].fork_left != &table.forks[1]
		|| table.philosophers[1].fork_right != &table.forks[0])
		return (fprintf(stderr, "philo[1] fork assignment wrong\n"), 1);
	table_free(&table);
	return (0);
}
