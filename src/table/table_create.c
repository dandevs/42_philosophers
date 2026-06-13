#include "lib.h"
#include "philosopher/utils.h"
#include <stdlib.h>

int table_create(t_table *table, t_config config)
{
	int	i;

	table->philosophers = malloc(sizeof(t_philosopher) * config.philo_count);
	i = 0;
	while (i < config.philo_count)
	{
		if (!philo_init(&table->philosophers[i], i))
			return (0);
		i++;
	}
	pthread_mutex_init(&table->printf_mutex, NULL);
	pthread_mutex_init(&table->mutex, NULL);
	table->alive = 1;
	table->config = config;
	return (1);
}