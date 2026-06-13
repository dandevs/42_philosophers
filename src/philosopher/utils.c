/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:22:32 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 07:18:36 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.h"

void	mutex_philo_table_lock(t_philosopher *philo)
{
	pthread_mutex_lock(&philo->mutex);
	pthread_mutex_lock(&philo->table->mutex);
}

void	mutex_philo_table_unlock(t_philosopher *philo)
{
	pthread_mutex_unlock(&philo->mutex);
	pthread_mutex_unlock(&philo->table->mutex);
}

void	with_philo_mutex(t_philosopher *philo, void (*func)(t_philosopher *))
{
	pthread_mutex_lock(&philo->mutex);
	func(philo);
	pthread_mutex_unlock(&philo->mutex);
}

void	mutex_forks_lock(t_philosopher *philo)
{
	lock_lock(philo->fork_left);
	lock_lock(philo->fork_right);
}

void	mutex_forks_unlock(t_philosopher *philo)
{
	lock_unlock(philo->fork_left);
	lock_unlock(philo->fork_right);
}

void	philo_mutexes_unlock(t_philosopher *philo)
{
	lock_unlock(philo->fork_left);
	lock_unlock(philo->fork_right);
}

int	philo_init(t_philosopher *philosophers, int index)
{
	philosophers->index = index;
	philosophers->alive = 1;
	philosophers->eat_count = 0;
	philosophers->done = 0;
	return (pthread_mutex_init(&philosophers->mutex, NULL) == 0);
}

void	philo_init_time(t_philosopher *philosophers)
{
	unsigned long t = get_time_ms();
	philosophers->time_last_meal = t;
	philosophers->time_began_eating = t;
	philosophers->time_began_sleep = t;
}