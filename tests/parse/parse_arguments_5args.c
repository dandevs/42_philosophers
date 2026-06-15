#include "lib.h"
#include <stdio.h>

int	main(void)
{
	char		*argv[] = {"prog", "5", "800", "200", "200"};
	t_config	config;
	int			ret;

	config = (t_config){0};
	ret = parse_arguments(5, argv, &config);
	if (!ret)
	{
		printf("parse_arguments(5 args) expected 1, got 0");
		return (1);
	}
	if (config.philo_count != 5)
	{
		printf("philo_count expected 5, got %d", config.philo_count);
		return (1);
	}
	if (config.time_to_die_ms != 800)
	{
		printf("time_to_die_ms expected 800, got %lu", config.time_to_die_ms);
		return (1);
	}
	if (config.time_to_eat_ms != 200)
	{
		printf("time_to_eat_ms expected 200, got %lu", config.time_to_eat_ms);
		return (1);
	}
	if (config.time_to_sleep_ms != 200)
	{
		printf("time_to_sleep_ms expected 200, got %lu",
			config.time_to_sleep_ms);
		return (1);
	}
	if (config.meals_required != -1)
	{
		printf("meals_required expected -1 (unset), got %d",
			config.meals_required);
		return (1);
	}
	return (0);
}
