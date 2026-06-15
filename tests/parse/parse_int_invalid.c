#include "lib.h"
#include "ctest.h"

int	parse_int(char *str, int *value);

int	main(void)
{
	char	*invalid[] = {"0", "abc", "-1", "+", ""};
	int		value;
	int		i;

	i = 0;
	while (i < (int)(sizeof(invalid) / sizeof(invalid[0])))
	{
		value = -123;
		ASSERT_FALSE(parse_int(invalid[i], &value));
		i++;
	}
	return (0);
}
