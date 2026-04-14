/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:43 by danimend          #+#    #+#             */
/*   Updated: 2026/04/14 21:03:39 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

static int	create_table(t_table *table, int count)
{
	table->philosophers = malloc(sizeof(t_philosopher) * count);
	table->forks = malloc(sizeof(t_fork) * count);
	if (!table->philosophers || !table->forks)
		return (free(table->philosophers), free(table->forks), 0);
	table->count = count;
	for (int i = 0; i < count; i++)
	{
		table->philosophers[i].fork_left = &table->forks[i];
		table->philosophers[i].fork_right = &table->forks[(i + 1) % count];
	}
	return (1);
}

static void	destroy_table(t_table *table)
{
	free(table->philosophers);
	free(table->forks);
}

int	main(int argc, char **argv)
{
	t_config	config;
	t_table		table;

	if (!parse_arguments(argc, argv, &config))
	{
		printf("Error: wrong number of arguments\n");
		printf("Usage: %s number_of_philosophers time_to_die "
			"time_to_eat time_to_sleep "
			"[number_of_times_each_philosopher_must_eat]\n", argv[0]);
		return (1);
	}
	if (!create_table(&table, config.philosophers_count))
		return (1);
	destroy_table(&table);
	return (0);
}
