#include "lib.h"
#include "table/table.h"
#include "mutex_utils.h"
#include "ctest.h"
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	t_table		table;
	t_config	config;
	int			saved;
	int			ret;

	config = (t_config){0};
	config.philo_count = 3;
	config.meals_required = -1;
	config.time_to_die_ms = 100;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	m_set_ulong(&table.philosophers[0].time_began_eating,
		get_time_ms() - 500, &table.philosophers[0].mutex);
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	ret = someone_died(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	ASSERT_TRUE(ret);
	ASSERT_EQ(m_get_int(&table.alive, &table.mutex), 0);
	table_free(&table);
	return (0);
}
