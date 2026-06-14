#include "fork.h"
#include <stdio.h>

int	main(void)
{
	t_fork	fork;
	int		ret;

	ret = fork_init(&fork);
	if (!ret)
	{
		printf("fork_init expected 1, got 0");
		return (1);
	}
	if (fork.available != 1)
	{
		printf("fork.available expected 1, got %d", fork.available);
		return (1);
	}
	pthread_mutex_lock(&fork.mutex);
	pthread_mutex_unlock(&fork.mutex);
	pthread_mutex_destroy(&fork.mutex);
	return (0);
}
