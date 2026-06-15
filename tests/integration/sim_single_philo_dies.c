#include "lib.h"
#include "table/table.h"
#include "ctest.h"
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	t_table			table;
	t_config		config;
	unsigned long	before;
	unsigned long	elapsed;
	int				saved;
	int				ret;

	config = (t_config){0};
	config.philo_count = 1;
	config.meals_required = -1;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	before = get_time_ms();
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	ret = table_main_routine(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	elapsed = get_time_ms() - before;
	ASSERT_TRUE(ret);
	ASSERT_TRUE(elapsed >= 790 && elapsed <= 3000);
	ASSERT_EQ(table.philosophers[0].eat_count, 0);
	table_free(&table);
	return (0);
}
