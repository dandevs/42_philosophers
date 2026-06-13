/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_main_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:15:29 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 08:34:03 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "philosopher/utils.h"
#include <unistd.h>
#include <stdio.h>

void	table_main_routine(t_table *table)
{
	table->start_time = get_time_ms();
	philo_init_time(table->philosophers);
}
