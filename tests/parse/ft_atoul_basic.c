#include "lib.h"
#include "ctest.h"

unsigned long	ft_atoul(char *str);

int	main(void)
{
	ASSERT_EQ(ft_atoul("0"), 0ul);
	ASSERT_EQ(ft_atoul("42"), 42ul);
	ASSERT_EQ(ft_atoul("0001"), 1ul);
	ASSERT_EQ(ft_atoul("123456"), 123456ul);
	return (0);
}
