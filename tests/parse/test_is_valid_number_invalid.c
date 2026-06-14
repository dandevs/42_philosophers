#include "lib.h"
#include <stdio.h>

int	is_valid_number(char *str);

int	main(void)
{
	char	*invalid[] = {"-", "abc", "12a", "4.2", "-1", "12 34", "++5",
		"1-2"};
	int		i;

	i = 0;
	while (i < (int)(sizeof(invalid) / sizeof(invalid[0])))
	{
		if (is_valid_number(invalid[i]))
		{
			printf("is_valid_number(\"%s\") expected 0, got 1",
				invalid[i]);
			return (1);
		}
		i++;
	}
	return (0);
}
