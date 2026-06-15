#include "lib.h"
#include <stdio.h>

unsigned long	ft_atoul(char *str);

int	main(void)
{
	if (ft_atoul("0") != 0)
	{
		printf("ft_atoul(\"0\") expected 0, got %lu", ft_atoul("0"));
		return (1);
	}
	if (ft_atoul("42") != 42)
	{
		printf("ft_atoul(\"42\") expected 42, got %lu", ft_atoul("42"));
		return (1);
	}
	if (ft_atoul("0001") != 1)
	{
		printf("ft_atoul(\"0001\") expected 1, got %lu", ft_atoul("0001"));
		return (1);
	}
	if (ft_atoul("123456") != 123456)
	{
		printf("ft_atoul(\"123456\") expected 123456, got %lu",
			ft_atoul("123456"));
		return (1);
	}
	return (0);
}
