import matplotlib.pyplot as plt
import os

def extract_best_cost(filepath):
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
            if lines:
                return float(lines[-1].strip())
    except Exception as e:
        print(f"⚠️ Błąd przy wczytywaniu pliku {filepath}: {e}")
    return None

# Liczba wątków: od 1 do 8
thread_counts = list(range(1, 9))
best_costs = []

for threads in thread_counts:
    folder = f"res{threads}"
    filepath = os.path.join(folder, "res.txt")
    cost = extract_best_cost(filepath)
    if cost is not None:
        best_costs.append(cost)
    else:
        best_costs.append(float('inf'))  # Na wypadek błędu, ustaw koszt na nieskończoność

# Rysowanie wykresu
plt.figure(figsize=(10, 5))
plt.scatter(thread_counts, best_costs, color='blue')
plt.xlabel("Liczba wątków")
plt.ylabel("Najlepszy znaleziony koszt")
plt.title("Najlepszy koszt w zależności od liczby wątków")
plt.grid(True)
plt.xticks(thread_counts)
plt.tight_layout()
plt.show()