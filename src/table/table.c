/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:15:29 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 04:58:35 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include <stdlib.h>

int	table_create(t_table *table, int count)
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

void	table_free(t_table *table)
{
	free(table->philosophers);
	free(table->forks);
}
