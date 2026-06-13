/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lock.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 00:00:00 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 00:31:25 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lock.h"

void	lock_init(t_lock *lock)
{
	pthread_mutex_init(&lock->mutex, NULL);
	lock->locked = 0;
}

void	lock_destroy(t_lock *lock)
{
	pthread_mutex_destroy(&lock->mutex);
}

void	lock_lock(t_lock *lock)
{
	pthread_mutex_lock(&lock->mutex);
	lock->locked = 1;
}

void	lock_unlock(t_lock *lock)
{
	if (lock->locked)
	{
		lock->locked = 0;
		pthread_mutex_unlock(&lock->mutex);
	}
}
