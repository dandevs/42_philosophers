/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_create.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 15:44:14 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 00:00:00 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "fork.h"
#include "philosopher/utils.h"
#include <stdlib.h>

static int	initialize_forks(t_table *table)
{
	int	i;

	table->forks = malloc(sizeof(t_fork) * table->config.philo_count);
	if (!table->forks)
		return (0);
	i = 0;
	while (i < table->config.philo_count)
	{
		if (!fork_init(&table->forks[i]))
		{
			free(table->forks);
			return (0);
		}
		i++;
	}
	return (1);
}

static void	init_philosopher(t_table *table, int i)
{
	t_philosopher	*philo;

	philo = &table->philosophers[i];
	philo->table = table;
	philo->fork_left = &table->forks[i];
	philo->fork_right = &table->forks[(i + 1) % table->config.philo_count];
	philo_init(philo, i);
}

int	table_create(t_table *table, t_config config)
{
	int	i;

	table->config = config;
	if (!initialize_forks(table))
		return (0);
	table->philosophers = malloc(sizeof(t_philosopher) * config.philo_count);
	if (!table->philosophers)
	{
		free(table->forks);
		return (0);
	}
	i = 0;
	while (i < config.philo_count)
	{
		init_philosopher(table, i);
		i++;
	}
	pthread_mutex_init(&table->printf_mutex, NULL);
	pthread_mutex_init(&table->mutex, NULL);
	table->alive = 1;
	return (1);
}
