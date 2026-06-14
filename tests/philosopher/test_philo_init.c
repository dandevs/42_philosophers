#include "philosopher/utils.h"
#include <stdio.h>

int	main(void)
{
	t_philosopher	philo;
	int				ret;

	ret = philo_init(&philo, 3);
	if (!ret)
	{
		printf("philo_init expected 1, got 0");
		return (1);
	}
	if (philo.index != 3)
	{
		printf("index expected 3, got %d", philo.index);
		return (1);
	}
	if (philo.eat_count != 0)
	{
		printf("eat_count expected 0, got %d", philo.eat_count);
		return (1);
	}
	if (philo.done != 0)
	{
		printf("done expected 0, got %d", philo.done);
		return (1);
	}
	if (philo.alive != 1)
	{
		printf("alive expected 1, got %d", philo.alive);
		return (1);
	}
	pthread_mutex_destroy(&philo.mutex);
	return (0);
}
