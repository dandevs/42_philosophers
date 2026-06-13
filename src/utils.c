/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 00:14:39 by danimend          #+#    #+#             */
/*   Updated: 2026/06/12 21:57:55 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "lib.h"
#include "mutex_utils.h"
#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>

unsigned long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int	is_valid_number(char *str)
{
	int	i;

	i = 0;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
		i++;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	return (str[i] == '\0');
}

int	parse_argument(char *str, int *value)
{
	if (!is_valid_number(str))
		return (0);
	*value = atoi(str);
	if (*value <= 0)
		return (0);
	return (1);
}

static int validate_arguments(t_config *config)
{
	if (config->philo_count <= 0)
		return (0);
	if (config->time_to_die_ms <= 0 || config->time_to_eat_ms <= 0)
		return (0);
	if (config->time_to_sleep_ms <= 0)
		return (0);
	if (config->meals_required != -1 && config->meals_required <= 0)
		return (0);
	return (1);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (argc != 5 && argc != 6)
		return (0);
	if (!parse_argument(argv[1], &config->philo_count))
		return (0);
	if (!parse_argument(argv[2], &config->time_to_die_ms))
		return (0);
	if (!parse_argument(argv[3], &config->time_to_eat_ms))
		return (0);
	if (!parse_argument(argv[4], &config->time_to_sleep_ms))
		return (0);
	if (argc == 6)
	{
		if (!parse_argument(argv[5], &config->meals_required))
			return (0);
	}
	else
		config->meals_required = -1;
	return (1);
}

void	philo_log(t_philosopher *philo, const char *message)
{
	if (!m_get_int(&philo->mutex, &philo->alive)
		|| !m_get_int(&philo->table->mutex, &philo->table->alive))
		return ;
	pthread_mutex_lock(&philo->table->printf_mutex);
	printf("%lu %d %s\n", get_time_ms() - philo->table->start_time,
		philo->index + 1, message);
	pthread_mutex_unlock(&philo->table->printf_mutex);
}

void	for_each(void *arr, int len, void (*func)(void *elem)) 
{
	int	i;

	i = 0;
	while (i < len) 
	{
		func((void *)((char *)arr + i * sizeof(t_philosopher)));
		i++;
	}
}

int		all(void *arr, int len, int (*predicate)(void *elem))
{
	int	i;

	i = 0;
	while (i < len) 
	{
		if (!predicate((void *)((char *)arr + i * sizeof(t_philosopher))))
			return (0);
		i++;
	}
	return (1);
}