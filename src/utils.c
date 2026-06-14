/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 00:14:39 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 15:26:30 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "mutex_utils.h"
#include <sys/time.h>
#include <stdio.h>

unsigned long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	philo_log(t_philosopher *philo, char *message)
{
	if (!m_get_int(&philo->table->alive, &philo->table->mutex))
		return ;
	pthread_mutex_lock(&philo->table->printf_mutex);
	printf("%lu %d %s\n", get_time_ms() - philo->table->start_time,
		philo->index + 1, message);
	pthread_mutex_unlock(&philo->table->printf_mutex);
}
