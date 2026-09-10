/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_utils.c                                      :+:    :+:    :+:    */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 06:01:27 by danimend          #+#    #+#             */
/*   Updated: 2026/09/10 17:44:32 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include <stdlib.h>

void	table_free(t_table *table)
{
	int	i;

	if (!table)
		return ;
	i = 0;
	while (table->forks && i < table->config.philo_count)
	{
		pthread_mutex_destroy(&table->forks[i].mutex);
		i++;
	}
	i = 0;
	while (table->philosophers && i < table->config.philo_count)
	{
		pthread_mutex_destroy(&table->philosophers[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&table->printf_mutex);
	pthread_mutex_destroy(&table->mutex);
	free(table->forks);
	free(table->philosophers);
}
