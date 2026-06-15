#include "lib.h"
#include "table/table.h"
#include "mutex_utils.h"
#include <stdio.h>

int	main(void)
{
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 2;
	config.meals_required = 2;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	m_set_int(&table.philosophers[0].done, 1, &table.philosophers[0].mutex);
	m_set_int(&table.philosophers[1].done, 1, &table.philosophers[1].mutex);
	if (!check_all_done(&table))
	{
		printf("check_all_done with all done expected 1, got 0");
		return (1);
	}
	table_free(&table);
	return (0);
}
