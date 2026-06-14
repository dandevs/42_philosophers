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

int	philo_init(t_philosopher *philosophers, int index)
{
	philosophers->index = index;
	philosophers->eat_count = 0;
	philosophers->done = 0;
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
		table->philosophers[i].time_began_eating = t;
		table->philosophers[i].start_time = t;
		i++;
	}
}
