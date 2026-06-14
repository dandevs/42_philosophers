#include "lib.h"
#include <stdio.h>

int	main(void)
{
	unsigned long	t1;
	unsigned long	t2;

	t1 = get_time_ms();
	t2 = get_time_ms();
	if (t2 < t1)
	{
		printf("get_time_ms not monotonic: %lu then %lu", t1, t2);
		return (1);
	}
	if (t2 - t1 > 1000)
	{
		printf("get_time_ms delta %lu too large", t2 - t1);
		return (1);
	}
	return (0);
}
