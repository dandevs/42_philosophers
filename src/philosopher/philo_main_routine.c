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

static int	is_running(t_philosopher *philo)
{
	return (m_get_int(&philo->table->alive, &philo->table->mutex));
}

static void	lock_forks(t_philosopher *philo)
{
	if (philo->index % 2 == 0)
	{
		pthread_mutex_lock(philo->fork_left);
		philo_log(philo, "has taken a fork");
		pthread_mutex_lock(philo->fork_right);
		philo_log(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(philo->fork_right);
		philo_log(philo, "has taken a fork");
		pthread_mutex_lock(philo->fork_left);
		philo_log(philo, "has taken a fork");
	}

	// pthread_mutex_lock(philo->fork_left);
	// philo_log(philo, "has taken a fork");
	// pthread_mutex_lock(philo->fork_right);
	// philo_log(philo, "has taken a fork");
}

static void	unlock_forks(t_philosopher *philo)
{
	pthread_mutex_unlock(philo->fork_left);
	pthread_mutex_unlock(philo->fork_right);
}

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo;
	t_config		config;

	philo = (t_philosopher *)arg;
	config = philo->table->config;

	while (1)
	{
		lock_forks(philo);
		m_set_ulong(&philo->time_began_eating, get_time_ms(), &philo->mutex);
		philo_log(philo, "is eating");
		usleep(config.time_to_eat_ms * 1000);
		unlock_forks(philo);
		m_set_ulong(&philo->time_last_meal, get_time_ms(), &philo->mutex);

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
