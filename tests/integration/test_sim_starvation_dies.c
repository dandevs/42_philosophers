#include "lib.h"
#include "table/table.h"
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
	config.philo_count = 4;
	config.meals_required = -1;
	config.time_to_die_ms = 310;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 100;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	before = get_time_ms();
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	ret = table_main_routine(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	elapsed = get_time_ms() - before;
	if (!ret)
	{
		printf("table_main_routine returned 0");
		return (1);
	}
	if (elapsed < 300 || elapsed > 3000)
	{
		printf("starvation death expected ~310ms, got %lums", elapsed);
		return (1);
	}
	table_free(&table);
	return (0);
}
