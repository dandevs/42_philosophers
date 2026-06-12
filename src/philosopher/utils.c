/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:22:32 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 03:34:51 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.h"
#include "lock.h"

void	mutex_philo_table_lock(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	pthread_mutex_lock(philo->table->mutex);
}

void	mutex_philo_table_unlock(t_philosopher *philo)
{
	pthread_mutex_unlock(philo->mutex);
	pthread_mutex_unlock(philo->table->mutex);
}

void	with_philo_lock(t_philosopher *philo, void (*func)(t_philosopher *))
{
	pthread_mutex_lock(philo->mutex);
	func(philo);
	pthread_mutex_unlock(philo->mutex);
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