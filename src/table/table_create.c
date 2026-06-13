#include "lib.h"
#include "philosopher/utils.h"
#include <stdlib.h>

int	table_create(t_table *table, t_config config)
{
	int	i;

	table->config = config;
	table->forks = malloc(sizeof(pthread_mutex_t) * config.philo_count);
	if (!table->forks)
		return (0);
	table->philosophers = malloc(sizeof(t_philosopher) * config.philo_count);
	if (!table->philosophers)
	{
		free(table->forks);
		return (0);
	}
	i = 0;
	while (i < config.philo_count)
	{
		table->philosophers[i].table = table;
		pthread_mutex_init(&table->forks[i], NULL);
		table->philosophers[i].fork_left = &table->forks[i];
		table->philosophers[i].fork_right = &table->forks[(i + 1) % config.philo_count];
		philo_init(&table->philosophers[i], i);
		i++;
	}
	pthread_mutex_init(&table->printf_mutex, NULL);
	pthread_mutex_init(&table->mutex, NULL);
	table->alive = 1;
	return (1);
}