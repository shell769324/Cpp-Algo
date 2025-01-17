import json
import matplotlib.pyplot as plt
import sys
import os
import numpy as np
import re

class TestResult:
    def __init__(self, name : str, element_name : str):
        self.name = name
        self.element_name = element_name
        self.datapoints : dict[str, dict[int, float]] = dict()
        self.rmses : dict[str, float] = dict()
        self.bigos : dict[str, str] = dict()
        self.bigo_coefficients : dict[str, float] = dict()

class ReportGenerator:
    def __init__(self, data_structure_name):
        self.suite_dict : dict[str, dict[str, TestResult]] = dict()
        self.data_structure_name = data_structure_name

    def __fill_suite_dict(self, data):
        for run in data:
            if "RMS" in run["name"] or "BigO" in run["name"]:
                continue
            split_array = re.sub(r'/iterations:\d+', '', run['name']).split("/")
            name = split_array[1]
            name = name[len(data_structure_name) + 1:]
            if name not in self.suite_dict:
                self.suite_dict[name] = dict()
            element_type = split_array[-2]
            if element_type not in self.suite_dict[name]:
                self.suite_dict[name][element_type] = TestResult(name, split_array[-2])
            if split_array[0] not in self.suite_dict[name][element_type].datapoints:
                self.suite_dict[name][element_type].datapoints[split_array[0]] = dict()
        for run in data:
            split_array = re.sub(r'/iterations:\d+', '', run['name']).split("/")
            name = split_array[1]
            name = name[len(data_structure_name) + 1:]
            if "RMS" in run["name"] or "BigO" in run["name"]:
                element_type = split_array[-1][:split_array[-1].rfind('_')]
            else:
                element_type = split_array[-2]
            if is_int(split_array[-1]):
                size = int(split_array[-1])
                time = float(run["cpu_time"])
                self.suite_dict[name][element_type].datapoints[split_array[0]][size] = time
            elif "RMS" in split_array[-1]:
                self.suite_dict[name][element_type].rmses[split_array[0]] = float(run["rms"])
            elif "BigO" in split_array[-1]:
                self.suite_dict[name][element_type].bigos[split_array[0]] = run["big_o"]
                self.suite_dict[name][element_type].bigo_coefficients[split_array[0]] = float(run["cpu_coefficient"])

    def __generate_time_complexity_comparison(self):
        for benchmark_name, suite in self.suite_dict.items():
            fig, axs = plt.subplots(1, len(suite), layout='constrained', figsize=(2.4 * len(suite), 2.4))
            fig.suptitle(benchmark_name, fontsize=10)
            count = 0
            group_list = list(suite.values())
            if len(suite) == 1:
                axs = [axs]
            for group, ax in zip(group_list, axs):
                sizes = list(next(iter(group.datapoints.values())).keys())
                sizes = sorted(sizes)
                max_len = 0
                ax.tick_params(axis='both', which='major', labelsize=7)
                for source_name, datapoints in group.datapoints.items():
                    coeff = float('%.3g' % group.bigo_coefficients[source_name])
                    label = f"{source_name} {coeff} {group.bigos[source_name]}"
                    ax.plot(sizes, [datapoints[size] for size in sizes], label=label, linestyle='--', marker='o')
                    max_len = max(max_len, len(source_name))
                ax.set_xscale('log')
                ax.set_xlabel("Input size", fontsize=9)
                ax.set_ylabel("CPU time in ns", fontsize=9)
                ax.set_title(group.element_name, fontsize=9)
                ax.legend(prop={'size': 6 if max_len > 10 else 7})
            plt.savefig(f"{script_dir}/generated/{data_structure_name}/{benchmark_name}")
            plt.close()

    def generate(self, data, generate_bigo_rmses=False):
        self.__fill_suite_dict(data)
        self.__generate_time_complexity_comparison()



def is_int(element: str) -> bool:
    #If you expect None to be passed:
    if element is None: 
        return False
    try:
        int(element)
        return True
    except ValueError:
        return False


if len(sys.argv) < 2:
    raise ValueError(f"You must provide the name of the data structure family. Valid types include deque, vector.")
data_structure_name = sys.argv[1]
script_dir = os.path.dirname(os.path.abspath(__file__))
file_path = f"{script_dir}/generated/{data_structure_name}/{data_structure_name}.json"
if not os.path.exists(file_path):
    executable_path = os.path.normpath(f"{script_dir}/../build/benchmark/cpp_algo_benchmark")
    json_path = f"{script_dir}/generated/{data_structure_name}/{data_structure_name}.json"
    benchmark_command = f"{executable_path} --benchmark_out={json_path} --benchmark_out_format=json"
    raise FileNotFoundError(f"File not found at {file_path}. You need to first run the following command from the root directory of this project. This will produce "
                            f"benchmark data used to generate the report.\n{benchmark_command}")
with open(file_path) as f:
    data = json.load(f)["benchmarks"]

generator = ReportGenerator(data_structure_name)
generator.generate(data)