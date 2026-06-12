/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lock.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 00:00:00 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 00:00:00 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCK_H
# define LOCK_H

# include <pthread.h>

typedef struct s_lock
{
	pthread_mutex_t	mutex;
	int				locked;
}	t_lock;

void	lock_init(t_lock *lock);
void	lock_destroy(t_lock *lock);
void	lock_lock(t_lock *lock);
void	lock_unlock(t_lock *lock);

#endif
