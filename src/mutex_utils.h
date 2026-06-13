/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mutex_utils.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 22:23:33 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 03:01:22 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MUTEX_UTILS_H
# define MUTEX_UTILS_H

# include <pthread.h>

int	set_int(pthread_mutex_t *mutex, int *ptr, int new_value);
int	get_int(pthread_mutex_t *mutex, int *ptr);
unsigned long	set_ulong(
			pthread_mutex_t *mutex,
			unsigned long *ptr,
			unsigned long new_value);
unsigned long	get_ulong(
			pthread_mutex_t *mutex,
			unsigned long *ptr);

#endif