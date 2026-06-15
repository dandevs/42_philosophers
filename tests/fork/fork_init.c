#include "fork.h"
#include "ctest.h"

int	main(void)
{
	t_fork	fork;

	ASSERT_TRUE(fork_init(&fork));
	ASSERT_EQ(fork.available, 1);
	pthread_mutex_lock(&fork.mutex);
	pthread_mutex_unlock(&fork.mutex);
	pthread_mutex_destroy(&fork.mutex);
	return (0);
}
