#include "lib.h"
#include "ctest.h"

int	parse_int(char *str, int *value);

int	main(void)
{
	int	value;

	value = -1;
	ASSERT_TRUE(parse_int("1", &value));
	ASSERT_EQ(value, 1);
	ASSERT_TRUE(parse_int("42", &value));
	ASSERT_EQ(value, 42);
	ASSERT_TRUE(parse_int("1000", &value));
	ASSERT_EQ(value, 1000);
	return (0);
}
