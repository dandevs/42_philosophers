#include "philosopher/utils.h"
#include "ctest.h"

int	main(void)
{
	t_philosopher	philo;

	ASSERT_TRUE(philo_init(&philo, 3));
	ASSERT_EQ(philo.index, 3);
	ASSERT_EQ(philo.eat_count, 0);
	ASSERT_EQ(philo.done, 0);
	ASSERT_EQ(philo.alive, 1);
	pthread_mutex_destroy(&philo.mutex);
	return (0);
}
