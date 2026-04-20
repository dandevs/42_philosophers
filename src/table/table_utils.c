/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 05:45:00 by danimend          #+#    #+#             */
/*   Updated: 2026/04/19 19:21:27 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include <stdlib.h>

int	table_create(t_table *table, int count)
{
	int	i;

	table->philosophers = malloc(sizeof(t_philosopher) * count);
	table->forks = malloc(sizeof(t_fork) * count);
	table->alive = 1;
	if (!table->philosophers || !table->forks)
		return (free(table->philosophers), free(table->forks), 0);
	table->count = count;
	pthread_mutex_init(&table->printf_mutex, NULL);
	pthread_mutex_init(&table->mutex, NULL);
	i = 0;
	while (i < count)
	{
		table->philosophers[i].index = i;
		table->philosophers[i].table = table;
		table->philosophers[i].fork_left = &table->forks[i];
		table->philosophers[i].fork_right = &table->forks[(i + 1) % count];
		pthread_mutex_init(&table->forks[i].mutex, NULL);
		pthread_mutex_init(&table->philosophers[i].mutex, NULL);
		i++;
	}

	i = 0;
	while (i < count)
	{
		if (pthread_create(&table->philosophers[i].thread, NULL,
			philo_main_routine, &table->philosophers[i]) != 0)
		{
			return (1);
		}
		i++;
	}
	return (1);
}

void	table_free(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->count)
	{
		pthread_mutex_destroy(&table->forks[i].mutex);
		pthread_mutex_destroy(&table->philosophers[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&table->printf_mutex);
	free(table->philosophers);
	free(table->forks);
}
