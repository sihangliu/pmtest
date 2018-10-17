import sys

NUM_CASES = 35

f = open(sys.argv[1])

rounds = []

count = 0
times = []
for line in f:
    time = int(line.split()[-1])
    times.append(time)
    count += 1
    if (count % NUM_CASES == 0):
        rounds.append(times)
        times = []

for i in range(0, NUM_CASES):
    sum = 0
    for j in range(0, len(rounds)):
        sum += rounds[j][i]
    print sum / len(rounds)
