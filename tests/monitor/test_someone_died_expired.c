#include "lib.h"
#include "table/table.h"
#include "mutex_utils.h"
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	t_table		table;
	t_config	config;
	int			saved;
	int			ret;
	int			alive_after;

	config = (t_config){0};
	config.philo_count = 3;
	config.meals_required = -1;
	config.time_to_die_ms = 100;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	m_set_ulong(&table.philosophers[0].time_began_eating,
		get_time_ms() - 500, &table.philosophers[0].mutex);
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	ret = someone_died(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	if (!ret)
	{
		printf("someone_died with expired meal expected 1, got 0");
		return (1);
	}
	alive_after = m_get_int(&table.alive, &table.mutex);
	if (alive_after != 0)
	{
		printf("table.alive expected 0 after death, got %d", alive_after);
		return (1);
	}
	table_free(&table);
	return (0);
}
