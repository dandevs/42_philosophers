/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 05:45:00 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 04:57:02 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "lock.h"
#include <stdlib.h>
#include <stdio.h>

static void init_forks(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->count)
	{
		lock_init(&table->forks[i]);
		i++;
	}
}

int	table_create(t_table *table, t_config config)
{
	int	i;

	table->philosophers = malloc(sizeof(t_philosopher) * config.philosophers_count);
	table->forks = malloc(sizeof(t_lock) * config.philosophers_count);
	table->philo_mutexes = malloc(sizeof(pthread_mutex_t) * config.philosophers_count);
	table->alive = 1;
	table->config = config;
	table->count = config.philosophers_count;
	table->threads_created = 0;
	table->mutex = malloc(sizeof(pthread_mutex_t));
	table->printf_mutex = malloc(sizeof(pthread_mutex_t));
	if (!table->philosophers || !table->forks || !table->philo_mutexes
		|| !table->mutex || !table->printf_mutex)
		return (free(table->philosophers), free(table->forks),
			free(table->philo_mutexes), free(table->mutex),
			free(table->printf_mutex), 0);
	init_forks(table);
	pthread_mutex_init(table->printf_mutex, NULL);
	pthread_mutex_init(table->mutex, NULL);
	i = 0;
	while (i < table->count)
	{
		t_philosopher *philo = &table->philosophers[i];
		philo->index = i;
		philo->table = table;
		philo->fork_left = &table->forks[i];
		philo->fork_right = &table->forks[(i + 1) % table->count];
		philo->time_last_meal = get_time_ms();
		philo->mutex = &table->philo_mutexes[i];
		philo->alive = 1;
		philo->eat_count = 0;
		philo->done = 0;
		pthread_mutex_init(philo->mutex, NULL);
		i++;
	}
	return (1);
}

int	table_start_philos(t_table *table)
{
	int	i;

	table->start_time = get_time_ms();
	i = 0;
	while (i < table->count)
	{
		table->philosophers[i].time_last_meal = get_time_ms();

		if (pthread_create(&table->philosophers[i].thread, NULL,
			philo_main_routine, &table->philosophers[i]) != 0)
		{
			table->threads_created = i;
			return (0);
		}
		i++;
	}
	table->threads_created = i;
	return (table->threads_created);
}

void	table_free(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->threads_created)
	{
		pthread_join(table->philosophers[i].thread, NULL);
		i++;
	}

	i = 0;
	while (i < table->count)
	{
		pthread_mutex_destroy(table->philosophers[i].mutex);
		i++;
	}
	pthread_mutex_destroy(table->mutex);
	pthread_mutex_destroy(table->printf_mutex);
	free(table->mutex);
	free(table->printf_mutex);
	free(table->philo_mutexes);
	i = 0;
	while (i < table->count)
	{
		lock_destroy(&table->forks[i]);
		i++;
	}
	free(table->forks);
	free(table->philosophers);
}
