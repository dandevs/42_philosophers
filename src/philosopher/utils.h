/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 20:00:05 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 21:51:11 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

# include "lib.h"

int		philo_init(t_philosopher *philosophers, int index);
void	philo_init_time(t_table *table);

#endif /* UTILS_H */
