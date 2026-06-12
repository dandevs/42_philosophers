/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 23:19:10 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 04:52:12 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIB_H
# define LIB_H
#include <pthread.h>
#include <sys/time.h>

#define POLLING_RATE 100

typedef struct s_table	t_table;
typedef struct s_lock	t_lock;

typedef struct s_philosopher
{
	t_lock			*fork_left;
	t_lock			*fork_right;
	pthread_mutex_t	*mutex;
	t_table			*table;
	pthread_t		thread;
	int				index;
	int				alive;
	int				state;
	int				eat_count;
	int				done;
	unsigned long	time_last_meal;
	unsigned long	time_began_eating;
	unsigned long	time_began_sleep;
}	t_philosopher;

typedef struct s_config
{
	int	philosophers_count;
	int	time_to_die_ms;
	int	time_to_eat_ms;
	int	time_to_sleep_ms;
	int	meals_required;
}	t_config;

typedef struct s_table
{
	t_philosopher	*philosophers;
	t_lock			*forks;
	pthread_mutex_t	*philo_mutexes;
	t_config		config;
	pthread_mutex_t	*printf_mutex;
	pthread_mutex_t	*mutex;
	unsigned long	start_time;
	int				count;
	int				alive;
	int				threads_created;
}	t_table;

unsigned long	get_time_ms(void);
int				is_valid_number(char *str);
int				parse_argument(char *str, int *value);
int				parse_arguments(int argc, char **argv, t_config *config);

void *philo_main_routine(void *arg);
void			philo_init_prerun(t_philosopher *philo);

void			for_each(void *arr, int len, void (*func)(void *elem));
int				all(void *arr, int len, int (*predicate)(void *elem));

#endif