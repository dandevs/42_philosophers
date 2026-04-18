/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:10 by danimend          #+#    #+#             */
/*   Updated: 2026/04/18 05:02:29 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIB_H
# define LIB_H
#include <pthread.h>

typedef struct s_fork
{
	pthread_mutex_t	lock;
}	t_fork;

typedef struct s_philosopher
{
	t_fork	*fork_left;
	t_fork	*fork_right;
}	t_philosopher;

typedef struct s_config
{
	int	philosophers_count;
	int	time_to_die;
	int	time_to_eat;
	int	time_to_sleep;
	int	meals_required;
}	t_config;

typedef struct s_table
{
	t_philosopher	*philosophers;
	t_fork			*forks;
	int				count;
}	t_table;

int	is_valid_number(char *str);
int	parse_argument(char *str, int *value);
int	parse_arguments(int argc, char **argv, t_config *config);

#endif