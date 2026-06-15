#include "lib.h"
#include "ctest.h"

int	main(void)
{
	unsigned long	t1;
	unsigned long	t2;

	t1 = get_time_ms();
	t2 = get_time_ms();
	ASSERT_GE(t2, t1);
	ASSERT_LE(t2 - t1, 1000ul);
	return (0);
}
