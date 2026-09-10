/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_monitor.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 06:11:27 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 07:03:27 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include "mutex_utils.h"
#include <unistd.h>
#include <stdio.h>

static int	check_death(t_table *table, int i)
{
	unsigned long	last_meal;
	unsigned long	elapsed;

	if (!m_get_int(&table->alive, &table->mutex))
		return (0);
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

int	check_all_done(t_table *table)
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

int	someone_died(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->config.philo_count)
	{
		if (check_death(table, i))
			return (1);
		i++;
	}
	return (0);
}

void	stop_threads(t_table *table)
{
	int	i;

	m_set_int(&table->alive, 0, &table->mutex);
	i = 0;
	while (i < table->config.philo_count)
	{
		m_set_int(&table->philosophers[i].alive, 0,
			&table->philosophers[i].mutex);
		i++;
	}
	i = 0;
	while (i < table->config.philo_count)
	{
		pthread_join(table->philosophers[i].thread, NULL);
		i++;
	}
}
