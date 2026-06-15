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
	int			i;

	config = (t_config){0};
	config.philo_count = 2;
	config.meals_required = 3;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	ret = table_main_routine(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	ASSERT_TRUE(ret);
	i = 0;
	while (i < config.philo_count)
	{
		ASSERT_EQ(m_get_int(&table.philosophers[i].eat_count,
				&table.philosophers[i].mutex), 3);
		i++;
	}
	table_free(&table);
	return (0);
}
