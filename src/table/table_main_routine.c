/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:15:29 by danimend          #+#    #+#             */
/*   Updated: 2026/06/10 03:50:02 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include <unistd.h>
#include <stdio.h>

static void	philo_is_dead(void *philo)
{
	t_philosopher *p = philo;

	pthread_mutex_lock(&p->mutex);
	p->alive = 0;
	pthread_mutex_unlock(&p->mutex);
}

void mark_all_philo_unalive(t_table *table)
{
	for (int i = 0; i < table->count; i++)
	{
		pthread_mutex_lock(&table->philosophers[i].mutex);
		table->philosophers[i].alive = 0;
		pthread_mutex_unlock(&table->philosophers[i].mutex);
	}
}

void	table_main_routine(t_table *table, t_config *config)
{
	int i;

	while (table->alive)
	{
		i = 0;

		while (i < table->count)
		{
			t_philosopher	philo = table->philosophers[i];
			pthread_mutex_lock(&philo.mutex);
			pthread_mutex_lock(&table->mutex);
			unsigned long	elapsed = get_time_ms() - philo.time_last_meal;

			if (philo.alive && elapsed > config->time_to_die)
			{
				table->alive = 0;
				break;
			}

			pthread_mutex_unlock(&philo.mutex);
			pthread_mutex_unlock(&table->mutex);
			i++;
		}

		usleep(POLLING_RATE);
	}
	// printf("Done\n");

	if (!table->alive)
		printf("A philosopher has died. Ending simulation.\n");

	// mark_all_philo_unalive(table);
}
