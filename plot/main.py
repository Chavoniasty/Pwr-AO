import matplotlib.pyplot as plt
import os

def load_data(filename):
    iterations = []
    costs = []
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) == 2:
                iterations.append(int(parts[0]))
                costs.append(float(parts[1]))
    return iterations, costs

plt.figure(figsize=(12, 6))

# 🔁 Wczytaj pliki thread0.txt do thread7.txt z folderu 'res/'
for i in range(8):
    filename = f"res{6}/thread{i}.txt"
    if os.path.exists(filename):
        iterations, costs = load_data(filename)
        plt.plot(iterations, costs, label=f"Wątek {i}")
    else:
        print(f"⚠️ Plik nie znaleziony: {filename}")

plt.xlabel("Iteracja")
plt.ylabel("Koszt")
plt.title("Wykres kosztu – każdy wątek z jednego uruchomienia (6 wątków)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
