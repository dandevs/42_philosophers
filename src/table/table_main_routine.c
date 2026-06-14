/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 15:44:11 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 00:00:00 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include "mutex_utils.h"
#include <unistd.h>
#include <stdio.h>

static int	create_threads(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->config.philo_count)
	{
		if (pthread_create(&table->philosophers[i].thread, NULL,
				philo_main_routine, &table->philosophers[i]))
			return (0);
		i++;
	}
	table->threads_created = 1;
	return (1);
}

int	table_main_routine(t_table *table)
{
	int	done;

	done = 0;
	philo_init_time(table);
	if (!create_threads(table))
		return (0);
	while (!done)
	{
		if (someone_died(table))
			done = 1;
		else if (check_all_done(table))
			done = 1;
		else
			usleep(POLLING_RATE);
	}
	stop_threads(table);
	return (1);
}
