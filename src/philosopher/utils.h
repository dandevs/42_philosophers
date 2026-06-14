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

# define PHILO_STATE_GET_FORKS 0
# define PHILO_STATE_EAT 1
# define PHILO_STATE_SLEEP 2

void	with_philo_mutex(t_philosopher *philo, void (*func)(t_philosopher *));
void	mutex_philo_table_lock(t_philosopher *philo);
void	mutex_philo_table_unlock(t_philosopher *philo);
int		philo_init(t_philosopher *philosophers, int index);
void	philo_init_time(t_table *table);

#endif /* UTILS_H */
