import subprocess
import matplotlib.pyplot as plt
import os
import re

DECIMS = [10, 12, 14, 16, 18, 20, 22, 24, 26]

avg = {}
avg["16"] = []
avg["24"] = []
avg["28"] = []

maxi = {}
maxi["16"] = []
maxi["24"] = []
maxi["28"] = []

cwd = os.getcwd()
for d in DECIMS:
    subprocess.run("sed -ri 's/#define N_ITERATIONS [0-9]+/#define N_ITERATIONS " + str(d) + "/' sin_test.cpp", shell=True)
    print(d) 
    os.chdir("../../../utils/generators")
    subprocess.run("sed -ri 's/#define TABLE_SIZE [0-9]+/#define TABLE_SIZE " + str(d) + "/' cordic_angle_gentable.cpp", shell=True)
    subprocess.run("make gen_cordic_table", shell=True)
    os.chdir(cwd)

    subprocess.run("shls clean && shls -a sw | tee outfile", shell=True)
    
    
#CORDIC 16 Count: 1257      Max diff: 0.001961      Avg diff: 0.000660
#CORDIC 24 Count: 1257      Max diff: 0.001934      Avg diff: 0.000626
#CORDIC 28 Count: 1257      Max diff: 0.001934      Avg diff: 0.000626    

    with open("outfile", "r") as outfile:
        for line in outfile:
            found_16 = re.match(r"CORDIC 16 Count: \d+\s+Max diff: (\d+.\d+)\s+Avg diff: (\d+.\d+)", line)
            found_24 = re.match(r"CORDIC 24 Count: \d+\s+Max diff: (\d+.\d+)\s+Avg diff: (\d+.\d+)", line)
            found_28 = re.match(r"CORDIC 28 Count: \d+\s+Max diff: (\d+.\d+)\s+Avg diff: (\d+.\d+)", line)

            if found_16:
                maxi["16"].append(float(found_16.group(1)))
                avg["16"].append(float(found_16.group(2)))
            if found_24:
                maxi["24"].append(float(found_24.group(1)))
                avg["24"].append(float(found_24.group(2)))
            if found_28:
                maxi["28"].append(float(found_28.group(1)))
                avg["28"].append(float(found_28.group(2)))
    
marker_size = 10
fig, axs = plt.subplots(2)


axs[0].scatter(DECIMS, avg["28"], s=marker_size)
axs[0].plot(DECIMS, avg["28"], "--", label="28 decimal bits", alpha=0.7)

axs[0].scatter(DECIMS, avg["24"], s=marker_size)
axs[0].plot(DECIMS, avg["24"], "--", label="24 decimal bits", alpha=0.7)

axs[0].scatter(DECIMS, avg["16"], s=marker_size)
axs[0].plot(DECIMS, avg["16"], "--", label="16 decimal bits", alpha=0.7)


axs[1].scatter(DECIMS, maxi["28"], s=marker_size)
axs[1].plot(DECIMS, maxi["28"], "--", label="28 decimal bits", alpha=0.7)

axs[1].scatter(DECIMS, maxi["24"], s=marker_size)
axs[1].plot(DECIMS, maxi["24"], "--", label="24 decimal bits", alpha=0.7)

axs[1].scatter(DECIMS, maxi["16"], s=marker_size)
axs[1].plot(DECIMS, maxi["16"], "--", label="16 decimal bits", alpha=0.7)

#axs[0].plot(DECIMS, avg["16"], label="16 decimal bits", linestyle="",  markersize=2, marker="o")


axs[0].legend(fontsize = "8")
axs[1].legend(fontsize = "8")

axs[0].grid()
axs[1].grid()
axs[0].set_ylabel("Average Error")
axs[1].set_ylabel("Max Error")
axs[0].set_xlabel("TABLE_SIZE")
axs[1].set_xlabel("TABLE_SIZE")
plt.show()    
