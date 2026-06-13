/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 00:14:39 by danimend          #+#    #+#             */
/*   Updated: 2026/06/13 15:26:30 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "lib.h"
#include "mutex_utils.h"
#include <stdlib.h>
#include <sys/time.h>
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

unsigned long	ft_atoul(char *str)
{
	unsigned long	result;
	int				i;

	i = 0;
	result = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result);
}

int	parse_ulong(char *str, unsigned long *value)
{
	if (!is_valid_number(str))
		return (0);
	*value = ft_atoul(str);
	if (*value == 0)
		return (0);
	return (1);
}

int	parse_int(char *str, int *value)
{
	unsigned long	tmp;

	if (!is_valid_number(str))
		return (0);
	tmp = ft_atoul(str);
	if (tmp == 0)
		return (0);
	*value = (int)tmp;
	return (1);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (argc != 5 && argc != 6)
		return (0);
	if (!parse_int(argv[1], &config->philo_count))
		return (0);
	if (!parse_ulong(argv[2], &config->time_to_die_ms))
		return (0);
	if (!parse_ulong(argv[3], &config->time_to_eat_ms))
		return (0);
	if (!parse_ulong(argv[4], &config->time_to_sleep_ms))
		return (0);
	if (argc == 6)
	{
		if (!parse_int(argv[5], &config->meals_required))
			return (0);
	}
	else
		config->meals_required = -1;
	return (1);
}

void	philo_log(t_philosopher *philo, char *message)
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