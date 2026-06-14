#include "lib.h"
#include <stdio.h>

int	is_valid_number(char *str);

int	main(void)
{
	char	*valid[] = {"42", "+42", " 42", "42 ", "  +42  ", "0001", "0"};
	int		i;

	i = 0;
	while (i < (int)(sizeof(valid) / sizeof(valid[0])))
	{
		if (!is_valid_number(valid[i]))
		{
			printf("is_valid_number(\"%s\") expected 1, got 0", valid[i]);
			return (1);
		}
		i++;
	}
	return (0);
}
