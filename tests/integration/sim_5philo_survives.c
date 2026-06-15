#include "lib.h"
#include "table/table.h"
#include "philosopher/utils.h"
#include "ctest.h"
#include <stdio.h>
#include <unistd.h>

#define WINDOW_MS 2000

int	main(void)
{
	t_table			table;
	t_config		config;
	unsigned long	start;
	int				saved;
	int				i;
	int				died;

	config = (t_config){0};
	config.philo_count = 5;
	config.meals_required = -1;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	philo_init_time(&table);
	saved = dup(1);
	freopen("/dev/null", "w", stdout);
	i = 0;
	while (i < config.philo_count)
	{
		pthread_create(&table.philosophers[i].thread, NULL,
			philo_main_routine, &table.philosophers[i]);
		i++;
	}
	start = get_time_ms();
	died = 0;
	while (get_time_ms() - start < WINDOW_MS)
	{
		if (someone_died(&table))
		{
			died = 1;
			break ;
		}
		usleep(POLLING_RATE);
	}
	stop_threads(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	ASSERT_FALSE(died);
	table_free(&table);
	return (0);
}
