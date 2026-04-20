/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 05:59:43 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 05:59:43 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo;
	t_table			*table;
	t_config		*config;

	philo = (t_philosopher *)arg;
	table = philo->table;
	config = table->config;
	while (1)
	{
		pthread_mutex_lock(&philo->fork_left->mutex);
		pthread_mutex_lock(&philo->fork_right->mutex);

		pthread_mutex_lock(&philo->mutex);
		if (philo->alive)
		{
			pthread_mutex_lock(&table->printf_mutex);
			printf("Philosopher %d is eating\n", philo->index);
			pthread_mutex_unlock(&table->printf_mutex);
		}
		pthread_mutex_unlock(&philo->mutex);

		pthread_mutex_unlock(&philo->fork_left->mutex);
		pthread_mutex_unlock(&philo->fork_right->mutex);

		pthread_mutex_lock(&philo->mutex);
		philo->time_last_meal = get_time_ms();
		pthread_mutex_unlock(&philo->mutex);

		pthread_mutex_lock(&philo->mutex);
		if (philo->alive)
			usleep(config->time_to_sleep * 1000);
		pthread_mutex_unlock(&philo->mutex);
	}

	return (NULL);
}

void	philo_init_prerun(t_philosopher *philo)
{
	philo->has_fork_left = 0;
	philo->has_fork_right = 0;
	philo->time_last_meal = get_time_ms();
	philo->alive = 1;
}