/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:10 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 21:40:26 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIB_H
# define LIB_H
# include <pthread.h>
# include <sys/time.h>
# include "fork.h"

# define POLLING_RATE 100

typedef struct s_table	t_table;

typedef struct s_philosopher
{
	pthread_mutex_t	mutex;
	t_fork			*fork_left;
	t_fork			*fork_right;
	t_table			*table;
	pthread_t		thread;
	int				index;
	int				eat_count;
	int				done;
	int				alive;
	unsigned long	time_began_eating;
	unsigned long	start_time;
}	t_philosopher;

typedef struct s_config
{
	int				philo_count;
	int				meals_required;
	unsigned long	time_to_die_ms;
	unsigned long	time_to_eat_ms;
	unsigned long	time_to_sleep_ms;
}	t_config;

typedef struct s_table
{
	t_philosopher	*philosophers;
	t_fork			*forks;
	pthread_mutex_t	printf_mutex;
	pthread_mutex_t	mutex;
	t_config		config;
	unsigned long	start_time;
	int				alive;
}	t_table;

unsigned long	get_time_ms(void);
void			philo_log(t_philosopher *philo, char *message);
void			*philo_main_routine(void *arg);
int				parse_arguments(int argc, char **argv, t_config *config);

#endif /* LIB_H */
