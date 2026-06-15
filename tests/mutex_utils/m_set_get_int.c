#include "mutex_utils.h"
#include "ctest.h"

int	main(void)
{
	pthread_mutex_t	mutex;
	int				value;

	pthread_mutex_init(&mutex, NULL);
	value = 0;
	ASSERT_EQ(m_set_int(&value, 42, &mutex), 42);
	ASSERT_EQ(m_get_int(&value, &mutex), 42);
	pthread_mutex_destroy(&mutex);
	return (0);
}
