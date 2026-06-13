/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 21:31:15 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 21:33:07 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef FORK_H
# define FORK_H
#include <pthread.h>

typedef struct s_fork
{
    pthread_mutex_t mutex;
    int available;
}   t_fork;

int fork_init(t_fork *fork);

#endif