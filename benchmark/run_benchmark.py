import sys
import os
import argparse
import subprocess
import json

def merge(src_path, dest_path):
    with open(src_path) as f:
        src = json.load(f)
    with open(dest_path) as f:
        dest = json.load(f)
    dest_dict = dict()
    for i, v in enumerate(dest["benchmarks"]):
        dest_dict[v['name']] = i
    for v in src["benchmarks"]:
        if v['name'] in dest_dict:
            i = dest_dict[v['name']]
            dest["benchmarks"][i] = v
        else:
            dest["benchmarks"].append(v)
    return dest

parser = argparse.ArgumentParser()
parser.add_argument('name', help="The name of the data structure to run benchmark on", choices=['vector', 'deque', 'map', 'set', 'range_query'])
parser.add_argument("-a", "--add", action=argparse.BooleanOptionalAction, help="add benchmark results to an existing file if true")
parser.add_argument('-d', "--dest", type=str, required=False, default="", help="the file to output the benchmark result to")
parser.add_argument('-r', "--regex_filter", type=str, required=False, help="benchmark regex filter")
parser.add_argument('-o', "--overwrite", action=argparse.BooleanOptionalAction, help="overwrite existing output file")

args = parser.parse_args()

script_dir = os.path.dirname(os.path.abspath(__file__))
data_structure_name = args.name
if args.dest:
    dest_path = f"{script_dir}/generated/{data_structure_name}/{parser.dest}.json"
else:
    dest_path = f"{script_dir}/generated/{data_structure_name}/{data_structure_name}.json"

if args.overwrite:
    if os.path.exists(dest_path):
        os.remove(dest_path)

if args.add:
    if not os.path.exists(dest_path):
        raise FileNotFoundError(f"{dest_path} doesn't exist. You can only add new benchmark results to an existing file.")
    command_output_path = f"{script_dir}/generated/{data_structure_name}/temp.json"
else:
    if os.path.exists(dest_path):
        raise FileExistsError(f"{dest_path} already exists. Please remove this file and retry. If you want to add benchmark results to this file, you can use the --add flag")
    command_output_path = dest_path

executable_path = os.path.normpath(f"{script_dir}/../build/benchmark/cpp_algo_benchmark")
command = [executable_path, f"--benchmark_out={command_output_path}", "--benchmark_out_format=json"]

if args.regex_filter:
    command.append(f"--benchmark_filter={args.regex_filter}")
else:
    command.append(f"--benchmark_filter={args.name}_*")

print(f"Running {' '.join(command)}")
subprocess.call(command)

if args.add:
    print(f"Merging {command_output_path} into {dest_path}")
    merged = merge(command_output_path, dest_path)
    print(f"Removing {command_output_path}")
    os.remove(command_output_path)
    with open(dest_path, 'w') as f:
        json.dump(merged, f, indent=4)
