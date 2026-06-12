#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	t_table table;
	t_config config = {
		.meals_required = 5,
		.philosophers_count = 10,
		.time_to_die_ms = 450,
		.time_to_eat_ms = 100,
		.time_to_sleep_ms = 300
	};

	if (!table_create(&table, config))
		return (fprintf(stderr, "table_create returned 0\n"), 1);

	table_main_routine(&table);
	return (0);
}
