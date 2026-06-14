/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:22:32 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 21:58:41 by danimend         ###   ########.fr       */
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

int	philo_init(t_philosopher *philosophers, int index)
{
	philosophers->index = index;
	philosophers->eat_count = 0;
	philosophers->done = 0;
	philosophers->schedule_locked = 0;
	philosophers->alive = 1;
	return (pthread_mutex_init(&philosophers->mutex, NULL) == 0);
}

void	philo_init_time(t_table *table)
{
	unsigned long	t;
	int				i;

	t = get_time_ms();
	table->start_time = t;
	i = 0;
	while (i < table->config.philo_count)
	{
		table->philosophers[i].time_last_meal = t;
		table->philosophers[i].time_began_eating = t;
		table->philosophers[i].time_began_sleep = t;
		table->philosophers[i].start_time = t;
		i++;
	}
}