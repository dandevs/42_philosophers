#include "lib.h"
#include "ctest.h"

int	is_valid_number(char *str);

int	main(void)
{
	char	*invalid[] = {"-", "abc", "12a", "4.2", "-1", "12 34", "++5",
		"1-2"};
	int		i;

	i = 0;
	while (i < (int)(sizeof(invalid) / sizeof(invalid[0])))
	{
		ASSERT_FALSE(is_valid_number(invalid[i]));
		i++;
	}
	return (0);
}
