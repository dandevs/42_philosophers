/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mutex_utils.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 22:23:33 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 00:00:00 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MUTEX_UTILS_H
# define MUTEX_UTILS_H

# include <pthread.h>

int				m_set_int(int *ptr, int new_value, pthread_mutex_t *mutex);
int				m_get_int(int *ptr, pthread_mutex_t *mutex);
unsigned long	m_set_ulong(unsigned long *ptr, unsigned long new_value,
					pthread_mutex_t *mutex);
unsigned long	m_get_ulong(unsigned long *ptr,
					pthread_mutex_t *mutex);

#endif /* MUTEX_UTILS_H */
