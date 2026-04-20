/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:40:00 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 16:07:39 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TABLE_H
# define TABLE_H

# include "lib.h"

int     table_create(t_table *table, int count);
void    table_free(t_table *table);
void	table_main_routine(t_table *table, t_config *config);

#endif
