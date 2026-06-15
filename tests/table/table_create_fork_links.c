#include "lib.h"
#include "table/table.h"
#include <stdio.h>

int	main(void)
{
	t_table		table;
	t_config	config;
	int			i;

	config = (t_config){0};
	config.philo_count = 4;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	i = 0;
	while (i < config.philo_count)
	{
		if (table.philosophers[i].fork_left != &table.forks[i])
		{
			printf("philo %d fork_left != &forks[%d]", i, i);
			return (1);
		}
		if (table.philosophers[i].fork_right
			!= &table.forks[(i + 1) % config.philo_count])
		{
			printf("philo %d fork_right mismatch", i);
			return (1);
		}
		i++;
	}
	if (table.philosophers[config.philo_count - 1].fork_right != &table.forks[0])
	{
		printf("last philo fork_right should wrap to &forks[0]");
		return (1);
	}
	table_free(&table);
	return (0);
}
