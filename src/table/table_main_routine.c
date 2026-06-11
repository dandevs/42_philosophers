/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:15:29 by danimend          #+#    #+#             */
/*   Updated: 2026/06/11 18:11:53 by danimend         ###   ########.fr       */
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

void	table_main_routine(t_table *table)
{
	int i;

	table_start_philos(table);
	while (1)
	{
		pthread_mutex_lock(&table->mutex);
		if (!table->alive)
			break ;

		i = 0;
		while (i < table->count)
		{
			t_philosopher	philo = table->philosophers[i];
			pthread_mutex_lock(&philo.mutex);
			unsigned long	elapsed = get_time_ms() - philo.time_last_meal;

			if (!philo.alive)
			{
				table->alive = 0;
				pthread_mutex_unlock(&philo.mutex);
				pthread_mutex_unlock(&table->mutex);
				break ;
			}

			pthread_mutex_unlock(&philo.mutex);
			pthread_mutex_unlock(&table->mutex);
			i++;
		}

		pthread_mutex_unlock(&table->mutex);
		usleep(POLLING_RATE);
	}

	if (!table->alive)
		printf("A philosopher has died. Ending simulation.\n");
}
