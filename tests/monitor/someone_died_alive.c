#include "lib.h"
#include "table/table.h"
#include "philosopher/utils.h"
#include "ctest.h"

int	main(void)
{
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 3;
	config.meals_required = -1;
	config.time_to_die_ms = 10000;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	philo_init_time(&table);
	ASSERT_FALSE(someone_died(&table));
	table_free(&table);
	return (0);
}
