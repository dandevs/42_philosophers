/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 06:04:27 by danimend          #+#    #+#             */
/*   Updated: 2026/06/14 06:39:27 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fork.h"

int	fork_init(t_fork *fork)
{
	fork->available = 1;
	return (pthread_mutex_init(&fork->mutex, NULL) == 0);
}
