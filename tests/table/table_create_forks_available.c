#include "lib.h"
#include "table/table.h"
#include "ctest.h"

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
	ASSERT_TRUE(table_create(&table, config));
	i = 0;
	while (i < config.philo_count)
	{
		ASSERT_EQ(table.forks[i].available, 1);
		i++;
	}
	table_free(&table);
	return (0);
}
