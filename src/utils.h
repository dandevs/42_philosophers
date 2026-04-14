/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :,      :,             */
/*                                                    :,     ,+;,             */
/*   By: danimend <danimend@student.42.fr>          ,+;:,   ,+;:;,            */
/*                                                ,+;;+:   ,+;;;+;,           */
/*   Updated: 2026/04/14 05:20:00 by danimend         ;+;;;;;;;;'              */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

typedef struct s_config	t_config;

int	is_valid_number(char *str);
int	parse_argument(char *str, int *value);
int	parse_arguments(int argc, char **argv, t_config *config);

#endif
