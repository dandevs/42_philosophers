#include "lib.h"
#include "ctest.h"

int	parse_ulong(char *str, unsigned long *value);

int	main(void)
{
	unsigned long	value;

	ASSERT_TRUE(parse_ulong("1", &value));
	ASSERT_EQ(value, 1ul);
	ASSERT_TRUE(parse_ulong("42", &value));
	ASSERT_EQ(value, 42ul);
	ASSERT_TRUE(parse_ulong("1000000", &value));
	ASSERT_EQ(value, 1000000ul);
	return (0);
}
