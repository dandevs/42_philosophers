/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :,      :,             */
/*                                                    :,     ,+;,             */
/*   By: danimend <danimend@student.42.fr>          ,+;:,   ,+;:;,             */
/*                                                ,+;;+:   ,+;;+;,            */
/*   Updated: 2026/04/14 05:20:00 by danimend         ;+;;;;;;;;'              */
/*                                                                            */
/* ************************************************************************** */

#include "lib.h"
#include <stdlib.h>

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
