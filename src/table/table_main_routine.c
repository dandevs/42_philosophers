/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 15:44:11 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 18:07:20 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include "mutex_utils.h"
#include <unistd.h>
#include <stdio.h>

static int	create_threads(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->config.philo_count)
	{
		if (pthread_create(&table->philosophers[i].thread, NULL,
				philo_main_routine, &table->philosophers[i]))
			return (0);
		pthread_detach(table->philosophers[i].thread);
		i++;
	}
	table->threads_created = 1;
	return (1);
}

static int	check_death(t_table *table, int i)
{
	unsigned long	last_meal;
	unsigned long	elapsed;

	last_meal = m_get_ulong(&table->philosophers[i].time_began_eating,
		&table->philosophers[i].mutex);
	elapsed = get_time_ms() - last_meal;
	if (elapsed >= table->config.time_to_die_ms)
	{
		m_set_int(&table->alive, 0, &table->mutex);
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
		if (!m_get_int(&table->philosophers[i].done,
			&table->philosophers[i].mutex))
			return (0);
		i++;
	}
	return (1);
}


void *table_scheduler(void *args)
{
	t_table *table = (t_table *)args;
	unsigned int round = 0;

	while (m_get_int(&table->alive, &table->mutex))
	{
		for (int i = 0; i < table->config.philo_count; i++)
		{
			t_philosopher *philo = &table->philosophers[i];
			if ((round + i) % 2 == 0)
			{
				// pthread_mutex_unlock(&philo->schedule_mutex);
				// usleep(1000);
				// pthread_mutex_lock(&philo->schedule_mutex);
			}

		}
		round++;
	}
}

int	table_main_routine(t_table *table)
{
	int	i;
	unsigned int round = 0;
	pthread_t scheduler_thread;

	philo_init_time(table);
	if (!create_threads(table))
		return (0);
	if (pthread_create(&scheduler_thread, NULL, table_scheduler, table) != 0)
		return (0);
	pthread_detach(scheduler_thread);
	while (1)
	{
		i = 0;
		while (i < table->config.philo_count)
		{
			if (check_death(table, i))
				return (1);
			i++;
		}
		if (check_all_done(table))
			return (1);
		usleep(POLLING_RATE);
		round++;
	}

	for (i = 0; i < table->config.philo_count; i++)
		m_set_int(&table->philosophers[i].alive, 0, &table->philosophers[i].mutex);

	return (1);
}
