/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 05:59:43 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 05:59:43 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include "philosopher/utils.h"
#include "mutex_utils.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

void	*philo_main_routine(void *arg)
{
	t_philosopher	*philo = (t_philosopher *)arg;

	while (1)
	{
		with_mutex(philo->mutex, philo, (void*)mutex_philo_table_lock);

		get_int(philo->mutex, &philo->alive);

		usleep(POLLING_RATE);
	}
	return (NULL);
}
