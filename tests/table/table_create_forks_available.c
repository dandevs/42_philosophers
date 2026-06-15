#include "lib.h"
#include "table/table.h"
#include <stdio.h>

int	main(void)
{
	t_table		table;
	t_config	config;
	int			i;

	config = (t_config){0};
	config.philo_count = 5;
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
		if (table.forks[i].available != 1)
		{
			printf("forks[%d].available expected 1, got %d", i,
				table.forks[i].available);
			return (1);
		}
		i++;
	}
	table_free(&table);
	return (0);
}
