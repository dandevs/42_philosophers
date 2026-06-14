#include "lib.h"
#include "table/table.h"
#include <stdio.h>

int	main(void)
{
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 3;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	table_free(&table);
	return (0);
}
