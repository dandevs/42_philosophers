/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:10 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 15:28:01 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIB_H
# define LIB_H
#include <pthread.h>
#include <sys/time.h>

# define POLLING_RATE 200

typedef struct s_table	t_table;
typedef struct s_lock	t_lock;

typedef struct s_philosopher
{
	pthread_mutex_t	mutex;
	pthread_mutex_t	*fork_left;
	pthread_mutex_t	*fork_right;
	t_table			*table;
	pthread_t		thread;
	int				index;
	int				alive;
	int				eat_count;
	int				done;
	unsigned long	time_last_meal;
	unsigned long	time_began_eating;
	unsigned long	time_began_sleep;
	unsigned long	start_time;
}	t_philosopher;

typedef struct s_config
{
	int				philo_count;
	int				meals_required;
	unsigned long	time_to_die_ms;
	unsigned long	time_to_eat_ms;
	unsigned long	time_to_sleep_ms;
	unsigned long	time_to_think_ms;
}	t_config;

typedef struct s_table
{
	t_philosopher	*philosophers;
	pthread_mutex_t	*forks;
	pthread_mutex_t	printf_mutex;
	pthread_mutex_t	mutex;
	t_config		config;
	unsigned long	start_time;
	int				alive;
	int				threads_created;
}	t_table;

unsigned long	get_time_ms(void);
void	philo_log(t_philosopher *philo, char *message);
void	*philo_main_routine(void *arg);
int		parse_arguments(int argc, char **argv, t_config *config);

#endif