/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:10 by danimend          #+#    #+#             */
/*   Updated: 2026/04/19 19:21:24 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIB_H
# define LIB_H
#include <pthread.h>
#include <sys/time.h>

#define POLLING_RATE 100

typedef struct s_fork
{
	pthread_mutex_t	mutex;
}	t_fork;

typedef struct s_table	t_table;

typedef struct s_philosopher
{
	t_fork			*fork_left;
	t_fork			*fork_right;
	t_table			*table;
	pthread_t		thread;
	pthread_mutex_t	mutex;
	int				index;
	int				has_fork_left;
	int				has_fork_right;
	int				alive;
	unsigned long	time_last_meal;
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
	t_config		*config;
	pthread_mutex_t	printf_mutex;
	pthread_mutex_t	mutex;
	int				count;
	int				alive;
}	t_table;

unsigned long	get_time_ms(void);
int				is_valid_number(char *str);
int				parse_argument(char *str, int *value);
int				parse_arguments(int argc, char **argv, t_config *config);

void			*philo_main_routine(void *arg);
void			philo_init_prerun(t_philosopher *philo);

void			for_each(void *arr, int len, void (*func)(void *elem));
int				all(void *arr, int len, int (*predicate)(void *elem));

#endif