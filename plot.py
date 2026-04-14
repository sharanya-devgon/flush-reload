import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import os


def parse_file(filename):
    results = {}
    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split(',')
            if len(parts) >= 3:
                try:
                    slot = int(parts[0])
                    time = int(parts[2])
                    results[slot] = time
                except:
                    pass
    return results


def plot_window(ax, sqr, mul, mod, plot_start, plot_end, threshold=100):
    plot_slots = list(range(plot_start, plot_end))

    for s in plot_slots:
        if s in sqr:
            t = sqr[s]
            ax.scatter(s, t, color='red' if t < threshold else '#ffaaaa',
                       s=40, zorder=3)
        if s in mod:
            t = mod[s]
            ax.scatter(s, t, color='blue' if t < threshold else '#aaaaff',
                       s=12, zorder=4)
        if s in mul:
            t = mul[s]
            if t < threshold:
                ax.scatter(s, t, facecolors='none', edgecolors='black',
                           linewidths=1.5, s=80, zorder=5)
            else:
                ax.scatter(s, t, facecolors='none', edgecolors='#aaaaaa',
                           linewidths=0.8, s=40, zorder=2)

    ax.axhline(y=threshold, color='black', linestyle='--', linewidth=1)
    ax.text(plot_start + 1, threshold + 5, 'Threshold', fontsize=8)
    ax.set_xlabel('Time Slot Number')
    ax.set_ylabel('Probe Time (cycles)')
    ax.set_ylim(0, 500)


def main():
    sqr_file = 'outputs/sqr_output.txt'
    mul_file = 'outputs/mul_output.txt'
    mod_file = 'outputs/modulo_output.txt'

    os.makedirs('outputs/processing', exist_ok=True)

    print("Loading files...")
    sqr = parse_file(sqr_file)
    mul = parse_file(mul_file)
    mod = parse_file(mod_file)

    all_slots = sorted(set(sqr.keys()) | set(mul.keys()) | set(mod.keys()))
    if not all_slots:
        print("No data found!")
        return

    start = all_slots[0]
    end = all_slots[-1]
    window = 100

    print(f"Slot range: {start} - {end}")
    print(f"Generating plots...")

    legend_elements = [
        Line2D([0], [0], marker='o', color='w', markerfacecolor='red',
               markersize=8, label='Square (HIT)'),
        Line2D([0], [0], marker='o', color='w', markerfacecolor='#ffaaaa',
               markersize=8, label='Square (miss)'),
        Line2D([0], [0], marker='o', color='w', markerfacecolor='blue',
               markersize=5, label='Modulo (HIT)'),
        Line2D([0], [0], marker='o', color='w', markerfacecolor='#aaaaff',
               markersize=5, label='Modulo (miss)'),
        Line2D([0], [0], marker='o', color='black', markerfacecolor='none',
               markersize=10, markeredgewidth=1.5, label='Multiply (HIT)'),
        Line2D([0], [0], marker='o', color='#aaaaaa', markerfacecolor='none',
               markersize=8, markeredgewidth=0.8, label='Multiply (miss)'),
    ]

    frame = 0
    current = start
    while current < end:
        plot_end = current + window
        fig, ax = plt.subplots(figsize=(16, 5))
        plot_window(ax, sqr, mul, mod, current, plot_end)

        sqr_hits = sum(1 for s in range(current, plot_end) if s in sqr and sqr[s] < 100)
        mul_hits = sum(1 for s in range(current, plot_end) if s in mul and mul[s] < 100)
        mod_hits = sum(1 for s in range(current, plot_end) if s in mod and mod[s] < 100)

        ax.set_title(f'Slots {current}-{plot_end}  |  sqr={sqr_hits}  mul={mul_hits}  mod={mod_hits}')
        ax.legend(handles=legend_elements, loc='upper right', fontsize=8)

        fname = f'outputs/processing/frame_{frame:04d}_slots_{current}-{plot_end}.png'
        plt.tight_layout()
        plt.savefig(fname, dpi=120)
        plt.close()

        if frame % 10 == 0:
            print(f"  frame {frame:04d} — slots {current}-{plot_end}")

        frame += 1
        current += window

    print(f"Done! {frame} frames saved to outputs/processing/")


if __name__ == '__main__':
    main()