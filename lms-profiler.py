#!/usr/bin/env python3
import sys
import csv
import numpy as np

def parse_profiling(file):
    current_time = 0
    begin_times = {}
    string_mapping = {}
    data = {}

    with open(file) as fp:
        reader = csv.reader(fp, delimiter=',', quotechar='\\')
        for row in reader:
            type = int(row[0])

            if type == 0 or type == 1:
                current_time = current_time + int(row[2])

            if type == 0:
                # Save the begin time in a temporary dict
                begin_times[int(row[1])] = current_time
            elif type == 1:
                id = int(row[1])
                label = string_mapping[id]
                if label not in data:
                    data[label] = []
                data[label].append((begin_times[id], current_time))
                del begin_times[id]
            elif type == 2:
                # save string mapping for later usage
                string_mapping[int(row[1])] = row[2]
    return data

def profiling_summary(data):
    sorted_keys = sorted(data.keys())

    key_len = max(map(len, sorted_keys))

    for key in sorted_keys:
        exec_times = [iv[1] - iv[0] for iv in data[key]]
        min_t = np.amin(exec_times)
        max_t = np.amax(exec_times)
        mean_t = np.mean(exec_times)
        std = np.std(exec_times)
        print("{0} \u00f8 {1:>9,.0f} \u00b1 {2:<7,.0f} [{3}, {4}] ({5})"
            .format(key.ljust(key_len), mean_t, std, min_t, max_t, len(exec_times)))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: lms-profiler.py <path>")
        sys.exit(1)
    else:
        data = parse_profiling(sys.argv[1])
        profiling_summary(data)
