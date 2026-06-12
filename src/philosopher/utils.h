/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 20:00:05 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 03:34:50 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_UTILS_H
# define PHILO_UTILS_H

# include "lib.h"

# define PHILO_STATE_GET_FORKS 0
# define PHILO_STATE_EAT 1
# define PHILO_STATE_SLEEP 2

void	with_philo_lock(t_philosopher *philo, void (*func)(t_philosopher *));
void	mutex_philo_table_lock(t_philosopher *philo);
void	mutex_philo_table_unlock(t_philosopher *philo);
void	mutex_forks_lock(t_philosopher *philo);
void	mutex_forks_unlock(t_philosopher *philo);
void	philo_mutexes_unlock(t_philosopher *philo);

#endif
