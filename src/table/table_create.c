/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_create.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 15:44:14 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 16:59:15 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "philosopher/utils.h"
#include <stdlib.h>

int	table_create(t_table *table, t_config config)
{
	int	i;

	table->config = config;
	table->forks = malloc(sizeof(pthread_mutex_t) * config.philo_count);
	// table->ph
	if (!table->forks)
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
		t_philosopher *philo = &table->philosophers[i];
		philo->table = table;
		pthread_mutex_init(&table->forks[i], NULL);
		philo->fork_left = &table->forks[i];
		philo->fork_right = &table->forks[(i + 1) % config.philo_count];
		philo_init(philo, i);
		pthread_mutex_lock(&philo->schedule_mutex);
		i++;
	}
	pthread_mutex_init(&table->printf_mutex, NULL);
	pthread_mutex_init(&table->mutex, NULL);
	table->alive = 1;
	return (1);
}