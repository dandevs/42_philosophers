/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danimend <danimend@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 00:14:39 by danimend          #+#    #+#             */
/*   Updated: 2026/04/19 06:56:41 by danimend         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "lib.h"
#include <stdlib.h>
#include <sys/time.h>

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
	if (config->philosophers_count <= 0)
		return (0);
	if (config->time_to_die <= 0 || config->time_to_eat <= 0)
		return (0);
	if (config->time_to_sleep <= 0)
		return (0);
	if (config->meals_required != -1 && config->meals_required <= 0)
		return (0);
	return (1);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (argc != 5 && argc != 6)
		return (0);
	if (!parse_argument(argv[1], &config->philosophers_count))
		return (0);
	if (!parse_argument(argv[2], &config->time_to_die))
		return (0);
	if (!parse_argument(argv[3], &config->time_to_eat))
		return (0);
	if (!parse_argument(argv[4], &config->time_to_sleep))
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

void	for_each(void *arr, int len, void (*func)(void *elem)) 
{
	int	i;

	i = 0;
	while (i < len) 
	{
		func((void *)((char *)arr + i * sizeof(void *)));
		i++;
	}
}

int		all(void *arr, int len, int (*predicate)(void *elem))
{
	int	i;

	i = 0;
	while (i < len) 
	{
		if (!predicate((void *)((char *)arr + i * sizeof(void *))))
			return (0);
		i++;
	}
	return (1);
}