#include "lib.h"
#include "table/table.h"
#include "mutex_utils.h"
#include "ctest.h"

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
	ASSERT_TRUE(table_create(&table, config));
	m_set_int(&table.philosophers[0].done, 1, &table.philosophers[0].mutex);
	ASSERT_FALSE(check_all_done(&table));
	table_free(&table);
	return (0);
}
