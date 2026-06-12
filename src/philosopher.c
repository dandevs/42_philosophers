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
#include "lock.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

/*
Required Events to Print

timestamp_in_ms X has taken a fork

When a philosopher picks up a fork (this will happen twice per eating cycle)


timestamp_in_ms X is eating

When a philosopher starts eating (after acquiring both forks)


timestamp_in_ms X is sleeping

When a philosopher finishes eating and starts sleeping


timestamp_in_ms X is thinking

When a philosopher wakes up and starts thinking


timestamp_in_ms X died

When a philosopher dies from starvation
*/

static void get_left_fork(t_philosopher *philo)
{
	lock_lock(philo->fork_left);
	pthread_mutex_lock(philo->mutex);
	pthread_mutex_lock(philo->table->printf_mutex);
	printf("%lu %d has taken a fork\n", get_time_ms() - philo->table->start_time, philo->index + 1);
	pthread_mutex_unlock(philo->table->printf_mutex);
	pthread_mutex_unlock(philo->mutex);
}

static void get_right_fork(t_philosopher *philo)
{
	lock_lock(philo->fork_right);
	pthread_mutex_lock(philo->mutex);
	pthread_mutex_lock(philo->table->printf_mutex);
	printf("%lu %d has taken a fork\n", get_time_ms() - philo->table->start_time, philo->index + 1);
	pthread_mutex_unlock(philo->table->printf_mutex);
	pthread_mutex_unlock(philo->mutex);
}

static void	begin_eating(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	philo->time_began_eating = get_time_ms();
	philo->time_last_meal = get_time_ms();
	philo->state = PHILO_STATE_EAT;
	pthread_mutex_unlock(philo->mutex);
	pthread_mutex_lock(philo->table->printf_mutex);
	printf("%lu %d is eating\n", get_time_ms() - philo->table->start_time, philo->index + 1);
	pthread_mutex_unlock(philo->table->printf_mutex);
	usleep(philo->table->config.time_to_eat_ms * 1000);
}

static int	simulation_running(t_table *table);

static void	begin_sleeping(t_philosopher *philo)
{
	unsigned long	sleep_start;

	pthread_mutex_lock(philo->mutex);
	philo->state = PHILO_STATE_SLEEP;
	pthread_mutex_unlock(philo->mutex);
	pthread_mutex_lock(philo->table->printf_mutex);
	printf("%lu %d is sleeping\n", get_time_ms() - philo->table->start_time, philo->index + 1);
	pthread_mutex_unlock(philo->table->printf_mutex);
	sleep_start = get_time_ms();
	while (get_time_ms() - sleep_start
		< (unsigned long)philo->table->config.time_to_sleep_ms)
	{
		if (!simulation_running(philo->table))
			break ;
		usleep(500);
	}
}

static void	begin_thinking(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	philo->state = PHILO_STATE_GET_FORKS;
	pthread_mutex_unlock(philo->mutex);
	pthread_mutex_lock(philo->table->printf_mutex);
	printf("%lu %d is thinking\n", get_time_ms() - philo->table->start_time, philo->index + 1);
	pthread_mutex_unlock(philo->table->printf_mutex);
}

static int	get_eat_count(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	int eat_count = philo->eat_count;
	pthread_mutex_unlock(philo->mutex);
	return eat_count;
}

static void	set_philo_done(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	philo->done = 1;
	pthread_mutex_unlock(philo->mutex);
}

static int	increment_eat_count(t_philosopher *philo)
{
	pthread_mutex_lock(philo->mutex);
	int count = ++philo->eat_count;
	pthread_mutex_unlock(philo->mutex);
	return count;
}

static int	simulation_running(t_table *table)
{
	int	running;

	pthread_mutex_lock(table->mutex);
	running = table->alive;
	pthread_mutex_unlock(table->mutex);
	return (running);
}

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo;
	t_table			*table;
	t_config		config;

	philo = (t_philosopher *)arg;
	table = philo->table;
	config = table->config;
	if (philo->index % 2 == 0)
		usleep(1000);
	while (simulation_running(table))
	{
		if (philo->index % 2 == 0)
		{
			get_left_fork(philo);
			get_right_fork(philo);
		}
		else
		{
			get_right_fork(philo);
			get_left_fork(philo);
		}
		begin_eating(philo);
		mutex_forks_unlock(philo);
		if (increment_eat_count(philo) >= table->config.meals_required)
		{
			set_philo_done(philo);
			break ;
		}
		begin_sleeping(philo);
		if (!simulation_running(table))
			break ;
		begin_thinking(philo);
	}
	return (NULL);
}


void	philo_init_prerun(t_philosopher *philo)
{
	philo->time_last_meal = get_time_ms();
	philo->alive = 1;
}