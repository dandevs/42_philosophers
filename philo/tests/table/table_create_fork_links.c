#include "lib.h"
#include "table/table.h"
#include "ctest.h"

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
	ASSERT_TRUE(table_create(&table, config));
	i = 0;
	while (i < config.philo_count)
	{
		ASSERT_EQ(table.philosophers[i].fork_left, &table.forks[i]);
		ASSERT_EQ(table.philosophers[i].fork_right,
			&table.forks[(i + 1) % config.philo_count]);
		i++;
	}
	ASSERT_EQ(table.philosophers[config.philo_count - 1].fork_right,
		&table.forks[0]);
	table_free(&table);
	return (0);
}
