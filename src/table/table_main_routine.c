#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include "mutex_utils.h"
#include <unistd.h>
#include <stdio.h>

static void	create_threads(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->config.philo_count)
	{
		pthread_create(&table->philosophers[i].thread, NULL,
			philo_main_routine, &table->philosophers[i]);
		pthread_detach(table->philosophers[i].thread);
		i++;
	}
	table->threads_created = 1;
}

static int	check_death(t_table *table, int i)
{
	unsigned long	last_meal;
	unsigned long	elapsed;

	last_meal = m_get_ulong(&table->philosophers[i].mutex,
		&table->philosophers[i].time_began_eating);
	elapsed = get_time_ms() - last_meal;
	if (elapsed >= table->config.time_to_die_ms)
	{
		m_set_int(&table->mutex, &table->alive, 0);
		pthread_mutex_lock(&table->printf_mutex);
		printf("%lu %d died\n", get_time_ms() - table->start_time,
			table->philosophers[i].index + 1);
		pthread_mutex_unlock(&table->printf_mutex);
		return (1);
	}
	return (0);
}

static int	check_all_done(t_table *table)
{
	int	i;

	if (table->config.meals_required == -1)
		return (0);
	i = 0;
	while (i < table->config.philo_count)
	{
		if (!m_get_int(&table->philosophers[i].mutex,
			&table->philosophers[i].done))
			return (0);
		i++;
	}
	return (1);
}

void	table_main_routine(t_table *table)
{
	int	i;

	philo_init_time(table);
	create_threads(table);
	while (1)
	{
		i = 0;
		while (i < table->config.philo_count)
		{
			if (check_death(table, i))
				return ;
			i++;
		}
		if (check_all_done(table))
			return ;
		usleep(POLLING_RATE);
	}
}
