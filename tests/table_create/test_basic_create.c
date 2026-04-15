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
	{
		printf("table_create returned 0\n");
		return (1);
	}
	if (table.count != count)
	{
		printf("expected count %d, got %d\n", count, table.count);
		return (1);
	}
	if (table.philosophers == NULL)
	{
		printf("philosophers is NULL\n");
		return (1);
	}
	if (table.forks == NULL)
	{
		printf("forks is NULL\n");
		return (1);
	}
	i = 0;
	while (i < count)
	{
		if (table.philosophers[i].fork_left != &table.forks[i])
		{
			printf("philo[%d].fork_left wrong\n", i);
			return (1);
		}
		if (table.philosophers[i].fork_right != &table.forks[(i + 1) % count])
		{
			printf("philo[%d].fork_right wrong\n", i);
			return (1);
		}
		i++;
	}
	table_destroy(&table);
	return (0);
}
