/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:40:00 by danimend          #+#    #+#             */
/*   Updated: 2026/06/11 16:01:41 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TABLE_H
# define TABLE_H

# include "lib.h"

int     table_create(t_table *table, t_config config, int count);
int     table_start_philos(t_table *table);
void    table_free(t_table *table);
void	table_main_routine(t_table *table);
void	mark_all_philo_unalive(t_table *table);

#endif
