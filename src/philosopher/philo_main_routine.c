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
#include "mutex_utils.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
	
static int	get_both_forks(t_philosopher *philo)
{
	if (philo->fork_left == philo->fork_right)
	{
		while (m_get_int(&philo->alive, &philo->mutex))
			usleep(POLLING_RATE);
		return (0);
	}

	while (m_get_int(&philo->alive, &philo->mutex))
	{
		pthread_mutex_lock(&philo->fork_left->mutex);
		pthread_mutex_lock(&philo->fork_right->mutex);

		if (philo->fork_left->available && philo->fork_right->available)
		{
			philo->fork_left->available = 0;
			philo->fork_right->available = 0;
			pthread_mutex_unlock(&philo->fork_left->mutex);
			pthread_mutex_unlock(&philo->fork_right->mutex);
			return (1);
		}

		pthread_mutex_unlock(&philo->fork_left->mutex);
		pthread_mutex_unlock(&philo->fork_right->mutex);
		usleep(POLLING_RATE);
	}

	return (0);
}

static void unlock_both_forks(t_philosopher *philo)
{
	if (philo->fork_left == philo->fork_right)
	{
		while (1)
			usleep(POLLING_RATE);
	}

	pthread_mutex_lock(&philo->fork_left->mutex);
	pthread_mutex_lock(&philo->fork_right->mutex);

	philo->fork_left->available = 1;
	philo->fork_right->available = 1;

	pthread_mutex_unlock(&philo->fork_left->mutex);
	pthread_mutex_unlock(&philo->fork_right->mutex);
}

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo;
	t_config		config;

	philo = (t_philosopher *)arg;
	config = philo->table->config;

	while (1)
	{
		get_both_forks(philo);
		m_set_ulong(&philo->time_began_eating, get_time_ms(), &philo->mutex);
		philo_log(philo, "is eating");
		usleep(config.time_to_eat_ms * 1000);
		m_set_ulong(&philo->time_last_meal, get_time_ms(), &philo->mutex);

		unlock_both_forks(philo);

		m_set_int(&philo->eat_count,
			m_get_int(&philo->eat_count, &philo->mutex) + 1, &philo->mutex);

		if (config.meals_required != -1
			&& m_get_int(&philo->eat_count, &philo->mutex)
			>= config.meals_required)
		{
			m_set_int(&philo->done, 1, &philo->mutex);
			return (NULL);
		}
		philo_log(philo, "is sleeping");
		usleep(config.time_to_sleep_ms * 1000);
		philo_log(philo, "is thinking");
	}
	return (NULL);
}
