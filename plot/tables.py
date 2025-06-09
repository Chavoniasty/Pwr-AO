import os

base_path = '.'
datasets = ['rbg358', 'ftv38', 'ft70', 'br17']
threads_list = [1, 2, 4, 8]
iterations_list = [10000, 25000, 50000, 100000]

def read_summary(path):
    try:
        with open(path, 'r') as f:
            line = f.readline().strip()
            # Format: "Average time: 258.00 ms, Average cost: 1591.00"
            parts = line.replace(',', '').split()
            avg_time = float(parts[2])
            avg_cost = float(parts[6])
            return avg_time, avg_cost
    except Exception:
        return None, None

results = []

for threads in threads_list:
    for dataset in datasets:
        for iters in iterations_list:
            summary_path = os.path.join(
                base_path,
                f"res{threads}",
                dataset,
                f"iter_{iters}",
                "summary.txt"
            )
            if not os.path.exists(summary_path):
                print(f"Nie znaleziono pliku: {summary_path}")
                continue
            avg_time, avg_cost = read_summary(summary_path)
            results.append({
                'dataset': dataset,
                'threads': threads,
                'iterations': iters,
                'avg_cost': avg_cost,
                'avg_time': avg_time
            })

for threads in threads_list:
    tex_filename = f'results_avg_{threads}threads.tex'
    with open(tex_filename, 'w') as tex_file:
        tex_file.write(f"% Tabela średnich wyników i czasów dla {threads} wątków\n")
        tex_file.write(r'''\begin{table}[h]
\centering
\begin{tabular}{|c|c|c|c|}
\hline
\textbf{Problem} & \textbf{Iteracje} & \textbf{Średni wynik} & \textbf{Średni czas [ms]} \\
\hline
''')

        filtered = [r for r in results if r['threads'] == threads]
        filtered = sorted(filtered, key=lambda x: (x['dataset'], x['iterations']))
        prev_problem = None

        for r in filtered:
            problem = r['dataset']
            iterations = r['iterations']
            avg_cost = r['avg_cost']
            avg_time = r['avg_time']

            avg_cost_str = f"{avg_cost:.2f}" if avg_cost is not None else '-'
            avg_time_str = f"{avg_time:.2f}" if avg_time is not None else '-'

            if prev_problem is not None and prev_problem != problem:
                tex_file.write(r"\hline" + "\n")

            tex_file.write(f"{problem} & {iterations} & {avg_cost_str} & {avg_time_str} \\\\\n")
            prev_problem = problem

        tex_file.write(r'''\hline
\end{tabular}
\caption{Średnie wyniki i czasy dla różnych problemów i iteracji przy liczbie wątków = ''' + f'{threads}' + r'''}
\label{tab:srednie_wyniki_{threads}}
\end{table}
''')

    print(f"Zapisano tabelę średnich wyników dla {threads} wątków do pliku: {tex_filename}")
