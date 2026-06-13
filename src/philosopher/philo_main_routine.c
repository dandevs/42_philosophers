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
	return (m_get_int(&philo->table->mutex, &philo->table->alive));
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

	if (philo->index % 2 == 0)
		usleep(250);
	while (is_running(philo))
	{
		lock_forks(philo);
		m_set_ulong(&philo->mutex, &philo->time_began_eating, get_time_ms());
		philo_log(philo, "is eating");
		usleep(config.time_to_eat_ms * 1000);
		unlock_forks(philo);
		m_set_ulong(&philo->mutex, &philo->time_last_meal, get_time_ms());
		m_set_int(&philo->mutex, &philo->eat_count,
			m_get_int(&philo->mutex, &philo->eat_count) + 1);
		if (config.meals_required != -1
			&& m_get_int(&philo->mutex, &philo->eat_count)
			>= config.meals_required)
		{
			m_set_int(&philo->mutex, &philo->done, 1);
			return (NULL);
		}
		philo_log(philo, "is sleeping");
		usleep(config.time_to_sleep_ms * 1000);
		philo_log(philo, "is thinking");
		usleep(500);
	}
	return (NULL);
}
