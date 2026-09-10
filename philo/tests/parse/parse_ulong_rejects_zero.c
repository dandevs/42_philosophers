#include "lib.h"
#include "ctest.h"

int	parse_ulong(char *str, unsigned long *value);

int	main(void)
{
	unsigned long	value;

	value = 999;
	ASSERT_FALSE(parse_ulong("0", &value));
	return (0);
}
