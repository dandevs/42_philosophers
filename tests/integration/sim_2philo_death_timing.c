#include "lib.h"
#include "table/table.h"
#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define CAPTURE_PATH "/tmp/philo_2philo_death.txt"
#define TTD 310

int	main(void)
{
	t_table			table;
	t_config		config;
	int				saved;
	int				ret;
	FILE			*f;
	char			line[256];
	unsigned long	death_t;
	int				found;
	int				id;

	config = (t_config){0};
	config.philo_count = 2;
	config.meals_required = -1;
	config.time_to_die_ms = TTD;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 100;
	ASSERT_TRUE(table_create(&table, config));
	saved = dup(1);
	freopen(CAPTURE_PATH, "w", stdout);
	ret = table_main_routine(&table);
	fflush(stdout);
	dup2(saved, 1);
	close(saved);
	ASSERT_TRUE(ret);
	f = fopen(CAPTURE_PATH, "r");
	ASSERT_NOT_NULL(f);
	death_t = 0;
	found = 0;
	id = 0;
	while (fgets(line, sizeof(line), f))
	{
		if (strstr(line, " died"))
		{
			sscanf(line, "%lu %d", &death_t, &id);
			found = 1;
		}
	}
	fclose(f);
	unlink(CAPTURE_PATH);
	ASSERT_TRUE(found);
	ASSERT_GE(death_t, (unsigned long)TTD);
	ASSERT_LE(death_t, (unsigned long)TTD + 10);
	table_free(&table);
	return (0);
}
