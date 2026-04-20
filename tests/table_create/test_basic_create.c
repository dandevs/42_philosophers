#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;
	int		count;
	int		i;

	count = 5;
	if (!table_create(&table, count))
		return (fprintf(stderr, "table_create returned 0\n"), 1);
	if (table.count != count)
		return (fprintf(stderr, "expected count %d, got %d\n", count, table.count), 1);
	if (table.philosophers == NULL)
		return (fprintf(stderr, "phlosophers is NULL\n"), 1);
	if (table.forks == NULL)
		return (fprintf(stderr, "forks is NULL\n"), 1);
	i = 0;
	while (i < count)
	{
		if (table.philosophers[i].fork_left != &table.forks[i])
			return (fprintf(stderr, "philo[%d].fork_left wrong\n", i), 1);
		if (table.philosophers[i].fork_right != &table.forks[(i + 1) % count])
			return (fprintf(stderr, "philo[%d].fork_right wrong\n", i), 1);
		i++;
	}
	table_free(&table);
	return (0);
}
