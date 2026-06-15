#include "lib.h"
#include "table/table.h"
#include "ctest.h"

int	main(void)
{
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 1;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	ASSERT_EQ(table.philosophers[0].fork_left,
		table.philosophers[0].fork_right);
	table_free(&table);
	return (0);
}
