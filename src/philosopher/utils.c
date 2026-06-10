/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:22:32 by danimend          #+#    #+#             */
/*   Updated: 2026/06/10 03:13:22 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.h"

void	mutex_philo_lock(t_philosopher *philo)
{
	pthread_mutex_lock(&philo->mutex);
	pthread_mutex_lock(&philo->table->mutex);
}

void	mutex_philo_release(t_philosopher *philo)
{
	pthread_mutex_unlock(&philo->mutex);
	pthread_mutex_unlock(&philo->table->mutex);
}

void	with_philo_lock(t_philosopher *philo, void (*func)(t_philosopher *))
{
	pthread_mutex_lock(&philo->mutex);
	func(philo);
	pthread_mutex_unlock(&philo->mutex);
}

void	take_left_fork(t_philosopher *philo)
{
	philo->fork_left->available = 0;
	philo->has_fork_left = 1;
}

void	take_right_fork(t_philosopher *philo)
{
	philo->fork_right->available = 0;
	philo->has_fork_right = 1;
}

void	release_left_fork(t_philosopher *philo)
{
	philo->fork_left->available = 1;
	philo->has_fork_left = 0;
}

void	release_right_fork(t_philosopher *philo)
{
	philo->fork_right->available = 1;
	philo->has_fork_right = 0;
}

void	release_both_forks(t_philosopher *philo)
{
	philo->fork_left->available = 1;
	philo->fork_right->available = 1;
	philo->has_fork_left = 0;
	philo->has_fork_right = 0;
}

void	mutex_forks_lock(t_philosopher *philo)
{
	pthread_mutex_lock(&philo->fork_left->mutex);
	pthread_mutex_lock(&philo->fork_right->mutex);
}

void	mutex_forks_release(t_philosopher *philo)
{
	pthread_mutex_unlock(&philo->fork_left->mutex);
	pthread_mutex_unlock(&philo->fork_right->mutex);
}