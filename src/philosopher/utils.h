/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 20:00:05 by danimend          #+#    #+#             */
/*   Updated: 2026/06/09 15:05:30 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_UTILS_H
# define PHILO_UTILS_H

# include "lib.h"

# define PHILO_STATE_GET_FORKS 0
# define PHILO_STATE_EAT 1
# define PHILO_STATE_SLEEP 2

void	with_philo_lock(t_philosopher *philo, void (*func)(t_philosopher *));
void	take_left_fork(t_philosopher *philo);
void	take_right_fork(t_philosopher *philo);
void	release_left_fork(t_philosopher *philo);
void	release_right_fork(t_philosopher *philo);
void	release_both_forks(t_philosopher *philo);
void	mutex_philo_lock(t_philosopher *philo);
void	mutex_philo_release(t_philosopher *philo);
void	mutex_forks_lock(t_philosopher *philo);
void	mutex_forks_release(t_philosopher *philo);

#endif
