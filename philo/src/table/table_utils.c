/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 06:01:27 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 07:24:27 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "table.h"
#include "lib.h"
#include <stdlib.h>

void	table_free(t_table *table)
{
	if (!table)
		return ;
	free(table->forks);
	free(table->philosophers);
}
