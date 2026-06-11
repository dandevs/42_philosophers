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
#include "philosopher/utils.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo;
	t_table			*table;
	t_config		config;
	int				state;

	philo = (t_philosopher *)arg;
	table = philo->table;
	config = table->config;
	state = PHILO_STATE_GET_FORKS;

	while (1)
	{
		mutex_philo_table_lock(philo);

		if (!philo->alive || !table->alive)
		{
			mutex_philo_table_unlock(philo);
			break ;
		}

		if (get_time_ms() > philo->time_last_meal + config.time_to_die)
		{
			philo->alive = 0;
			mutex_philo_table_unlock(philo);
			break ;
		}

		if (state == PHILO_STATE_GET_FORKS)
		{
			mutex_forks_lock(philo);
			if (!philo->has_fork_left && philo->fork_left->available)
				take_left_fork(philo);
			if (!philo->has_fork_right && philo->fork_right->available)
				take_right_fork(philo);
			mutex_forks_unlock(philo);

			if (philo->has_fork_left && philo->has_fork_right)
			{
				philo->time_began_eating = get_time_ms();
				state = PHILO_STATE_EAT;
			}
		}

		if (state == PHILO_STATE_EAT)
		{
			if (get_time_ms() >= philo->time_began_eating + config.time_to_eat)
			{
				mutex_forks_lock(philo);
				release_both_forks(philo);
				state = PHILO_STATE_SLEEP;
				mutex_forks_unlock(philo);
				philo->time_began_sleep = get_time_ms();
			}
		}

		if (state == PHILO_STATE_SLEEP)
		{
			if (get_time_ms() >= philo->time_began_sleep + config.time_to_sleep)
			{
				philo->time_last_meal = get_time_ms();
				state = PHILO_STATE_GET_FORKS;
			}
		}

		mutex_philo_table_unlock(philo);
		usleep(POLLING_RATE);
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