#include "lib.h"
#include <stdio.h>

int	parse_int(char *str, int *value);

int	main(void)
{
	char	*invalid[] = {"0", "abc", "-1", "+", ""};
	int		value;
	int		ret;
	int		i;

	i = 0;
	while (i < (int)(sizeof(invalid) / sizeof(invalid[0])))
	{
		value = -123;
		ret = parse_int(invalid[i], &value);
		if (ret)
		{
			printf("parse_int(\"%s\") expected 0, got 1 (value %d)",
				invalid[i], value);
			return (1);
		}
		i++;
	}
	return (0);
}
