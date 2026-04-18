/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 22:40:00 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 04:53:38 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TABLE_H
# define TABLE_H

# include "lib.h"

int	table_create(t_table *table, int count);
void	table_free(t_table *table);

#endif
