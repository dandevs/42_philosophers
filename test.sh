#!/usr/bin/env bash
# Philosophers test runner
# Usage: ./test.sh [test_name]

PHILO=./philo
PASS=0
FAIL=0

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

run_test() {
	local name="$1"
	local args="$2"
	local expect_death="$3"
	local timeout="$4"

	printf "${CYAN}[TEST]${NC} %-40s" "$name"
	output=$(timeout "$timeout" "$PHILO" $args 2>&1)
	local exit_code=$?

	if [ $exit_code -eq 124 ]; then
		if [ "$expect_death" = "any" ]; then
			echo -e " ${GREEN}PASS${NC}"
			PASS=$((PASS + 1))
			return
		fi
		echo -e " ${RED}TIMEOUT${NC}"
		FAIL=$((FAIL + 1))
		return
	fi
	if [ $exit_code -eq 139 ]; then
		echo -e " ${RED}SEGFAULT${NC}"
		FAIL=$((FAIL + 1))
		return
	fi
	if [ "$expect_death" != "err" ] && [ $exit_code -ne 0 ]; then
		echo -e " ${RED}EXIT $exit_code${NC}"
		FAIL=$((FAIL + 1))
		return
	fi
	if [ "$expect_death" = "err" ] && [ $exit_code -eq 0 ]; then
		echo -e " ${RED}SHOULD HAVE FAILED${NC}"
		FAIL=$((FAIL + 1))
		return
	fi
	if echo "$output" | grep -qi "Error"; then
		if [ "$expect_death" != "err" ]; then
			echo -e " ${RED}ERROR MSG${NC}"
			FAIL=$((FAIL + 1))
			return
		fi
	else
		if [ "$expect_death" = "err" ]; then
			echo -e " ${RED}MISSING ERROR${NC}"
			FAIL=$((FAIL + 1))
			return
		fi
	fi
	if [ "$expect_death" = "any" ]; then
		:
	elif [ "$expect_death" = "yes" ]; then
		if ! echo "$output" | grep -q "died"; then
			echo -e " ${RED}EXPECTED DEATH${NC}"
			FAIL=$((FAIL + 1))
			return
		fi
		local last_line=$(echo "$output" | tail -1)
		if ! echo "$last_line" | grep -q "died"; then
			echo -e " ${RED}DEATH NOT LAST${NC}"
			FAIL=$((FAIL + 1))
			return
		fi
		if echo "$output" | grep -q "is eating" 2>/dev/null; then
			local last_eat=$(echo "$output" | grep "is eating" | tail -1 | awk '{print $1}')
			local death_t=$(echo "$last_line" | awk '{print $1}')
			if [ -n "$last_eat" ] && [ -n "$death_t" ] && [ "$last_eat" -gt "$death_t" ] 2>/dev/null; then
				echo -e " ${RED}ATE AFTER DEATH${NC}"
				FAIL=$((FAIL + 1))
				return
			fi
		fi
	elif [ "$expect_death" = "no" ]; then
		if echo "$output" | grep -q "died"; then
			echo -e " ${RED}UNEXPECTED DEATH${NC}"
			FAIL=$((FAIL + 1))
			return
		fi
	fi
	if [ "$expect_death" != "err" ] && [ -n "$output" ]; then
		local prev_t=-1
		while IFS= read -r line; do
			case "$line" in
				[0-9]*)
					local ts=$(echo "$line" | awk '{print $1}')
					if [ "$ts" -lt "$prev_t" ] 2>/dev/null; then
						echo -e " ${RED}BAD TIMESTAMP${NC}"
						FAIL=$((FAIL + 1))
						return
					fi
					prev_t=$ts
					;;
			esac
		done <<< "$output"
	fi
	echo -e " ${GREEN}PASS${NC}"
	PASS=$((PASS + 1))
}

echo "=== Death Tests ==="
run_test "single philo dies"              "1 800 200 200"      yes 10
run_test "single philo dies quick"        "1 200 100 100"      yes 10
run_test "4 philos, die < eat+sleep"      "4 310 200 100"      yes 10
run_test "4 philos, fast death"           "4 100 200 200"      yes 10
run_test "3 philos, starve"               "5 400 200 200"      yes 10
run_test "single philo, one fork"         "1 400 100 100"      yes 10

echo ""
echo "=== No Death w/ Meals Required ==="
run_test "2 philos, 5 meals"              "2 2000 200 200 5"   no  15
run_test "3 philos, 3 meals"              "3 2000 200 200 3"   no  15
run_test "4 philos, 4 meals"              "4 2000 200 200 4"   no  20
run_test "5 philos, 3 meals"              "5 2000 200 200 3"   no  20
run_test "3 philos, 1 meal (fast)"        "3 800 200 200 1"    no  10
run_test "2 philos, 10 meals"             "2 4000 200 200 10"  no  25
run_test "4 philos, 2 meals"              "4 4000 200 200 2"   no  15

echo ""
echo "=== Stress Tests (3s sample) ==="
run_test "20 philos, no crash"            "20 800 200 200"     any 6
run_test "50 philos, no crash"            "50 800 200 200"     any 6
run_test "100 philos, no crash"           "100 800 200 200"    any 6

echo ""
echo "=== Error Handling ==="
run_test "no args"                        ""                   err 3
run_test "missing arg"                    "5"                  err 3
run_test "partial args"                   "5 800"              err 3
run_test "invalid count"                  "0 800 200 200"      err 3

echo ""
echo -e "${GREEN}PASS: $PASS${NC}  ${RED}FAIL: $FAIL${NC}  TOTAL: $((PASS + FAIL))"
[ "$FAIL" -eq 0 ]
