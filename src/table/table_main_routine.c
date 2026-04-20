/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:15:29 by danimend          #+#    #+#             */
/*   Updated: 2026/04/19 22:49:01 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include <unistd.h>
#include <stdio.h>

static void	philo_is_dead(void *philo)
{
	t_philosopher *p = philo;

	pthread_mutex_lock(&p->mutex);
	p->alive = 0;
	pthread_mutex_unlock(&p->mutex);
}

void	table_main_routine(t_table *table, t_config *config)
{
	for (int i = 0; i < table->count; i++)
		philo_init_prerun(&table->philosophers[i]);

	while (1)
	{
		int i;
		int dead;

		i = 0;
		while (i < table->count)
		{
			t_philosopher	philo = table->philosophers[i];
			unsigned long	elapsed = get_time_ms() - philo.time_last_meal;

			if (philo.alive)
			{
				if (elapsed > config->time_to_die)
				{
					table->alive = 0;
					break;
				}
			}

			i++;
		}

		usleep(POLLING_RATE);
	}
}
