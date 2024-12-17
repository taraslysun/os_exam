import subprocess
import os
import argparse
import time
def create_config_file(mode, num_processes, shared_mem_name=None, ip_addresses=None):
    with open("config.txt", "w") as f:
        f.write(f"{mode}\n")
        f.write(f"{num_processes}\n")
        if mode == 0:
            f.write(f"{shared_mem_name}\n")
        else:
            for ip in ip_addresses:
                f.write(f"{ip}\n")

def launch_processes(exe, num_processes, config_file):
    processes = []
    for rank in range(num_processes):
        time.sleep(1)
        cmd = [exe, str(rank), config_file]
        proc = subprocess.Popen(cmd)
        processes.append(proc)
    for proc in processes:
        proc.wait()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=str, default="./bin/circle_numbers")
    parser.add_argument("--num_processes", type=int, default=4)
    parser.add_argument("--mode", type=int, default=0)
    parser.add_argument("--cfg", type=str, default="cfg/local.cfg")
    args = parser.parse_args()
    if args.mode == 0:
        shared_mem_name = "/mpi_shared_memory"
    else:
        ip_addresses = ["127.0.0.1"] * args.num_processes 
    launch_processes(args.exe, args.num_processes, args.cfg)

