#include "lib.h"
#include "table/table.h"
#include "philosopher/utils.h"
#include <stdio.h>

int	main(void)
{
	t_table			table;
	t_config		config;
	unsigned long	before;
	unsigned long	after;
	int				i;

	config = (t_config){0};
	config.philo_count = 3;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	before = get_time_ms();
	philo_init_time(&table);
	after = get_time_ms();
	if (table.start_time < before || table.start_time > after)
	{
		printf("start_time %lu not in [%lu, %lu]", table.start_time, before,
			after);
		return (1);
	}
	i = 0;
	while (i < config.philo_count)
	{
		if (table.philosophers[i].time_began_eating != table.start_time)
		{
			printf("philo %d time_began_eating %lu != start_time %lu", i,
				table.philosophers[i].time_began_eating, table.start_time);
			return (1);
		}
		if (table.philosophers[i].start_time != table.start_time)
		{
			printf("philo %d start_time mismatch", i);
			return (1);
		}
		i++;
	}
	table_free(&table);
	return (0);
}
