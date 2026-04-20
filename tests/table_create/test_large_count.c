#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;
	int		count;
	int		spot_checks[] = {0, 1, 42, 99, 199};
	int		n_checks;
	int		i;
	int		idx;

	count = 200;
	n_checks = 5;
	if (!table_create(&table, count))
		return (fprintf(stderr, "table_create returned 0\n"), 1);
	if (table.count != count)
		return (fprintf(stderr, "expected count %d, got %d\n", count, table.count), 1);
	i = 0;
	while (i < n_checks)
	{
		idx = spot_checks[i];
		if (table.philosophers[idx].fork_left != &table.forks[idx])
			return (fprintf(stderr, "philo[%d].fork_left wrong\n", idx), 1);
		if (table.philosophers[idx].fork_right
			!= &table.forks[(idx + 1) % count])
			return (fprintf(stderr, "philo[%d].fork_right wrong\n", idx), 1);
		i++;
	}
	table_free(&table);
	return (0);
}
