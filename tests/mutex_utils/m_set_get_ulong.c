#include "mutex_utils.h"
#include "ctest.h"

int	main(void)
{
	pthread_mutex_t	mutex;
	unsigned long	value;

	pthread_mutex_init(&mutex, NULL);
	value = 0;
	ASSERT_EQ(m_set_ulong(&value, 999999ul, &mutex), 999999ul);
	ASSERT_EQ(m_get_ulong(&value, &mutex), 999999ul);
	pthread_mutex_destroy(&mutex);
	return (0);
}
