#include "table.h"
#include "lib.h"
#include <stdlib.h>

void	table_free(t_table *table)
{
	if (!table)
		return ;

	pthread_mutex_destroy(&table->printf_mutex);
	
	for (int i = 0; i < table->config.philo_count; i++)
	{
		pthread_mutex_destroy(&table->forks[i]);
		pthread_mutex_destroy(&table->philosophers[i].mutex);
	}

	free(table->forks);
	free(table->philosophers);
}