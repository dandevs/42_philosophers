/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:43 by danimend          #+#    #+#             */
/*   Updated: 2026/04/19 17:34:27 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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
	if (!table_create(&table, config.philosophers_count))
		return (1);
	table_free(&table);

	while (1)
	{
		for (int i = 0; i < table.count; i++)
		{
			
		}

		usleep(100);
	}

	return (0);
}
