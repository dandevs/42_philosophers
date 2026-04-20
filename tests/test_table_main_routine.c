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
		.philosophers_count = 3,
		.time_to_die = 1000,
		.time_to_eat = 200,
		.time_to_sleep = 300
	};

	if (!table_create(&table, 1))
		return (fprintf(stderr, "table_create returned 0\n"), 1);

	table_main_routine(&table, &config);
	table_free(&table);
	return (0);
}
