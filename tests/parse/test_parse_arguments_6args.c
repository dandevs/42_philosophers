#include "lib.h"
#include <stdio.h>

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200", "7"};
	t_config	config;
	int			ret;

	config = (t_config){0};
	ret = parse_arguments(6, argv, &config);
	if (!ret)
	{
		printf("parse_arguments(6 args) expected 1, got 0");
		return (1);
	}
	if (config.meals_required != 7)
	{
		printf("meals_required expected 7, got %d", config.meals_required);
		return (1);
	}
	if (config.philo_count != 5)
	{
		printf("philo_count expected 5, got %d", config.philo_count);
		return (1);
	}
	return (0);
}
