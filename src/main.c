/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:43 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 20:07:31 by danimend         ###   ########.fr       */
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
		printf("Error: invalid arguments\n");
		return (1);
	}
	if (!table_create(&table, config))
		return (1);
	if (!table_main_routine(&table))
		return (0);
	table_free(&table);
	return (0);
}
