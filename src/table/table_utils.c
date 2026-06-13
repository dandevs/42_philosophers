/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 05:45:00 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 20:07:37 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include "lock/lock.h"
#include <stdlib.h>
#include <stdio.h>

static void init_forks(t_table *table)
{
}

int	table_create(t_table *table, t_config config)
{
	init_forks(table);
}