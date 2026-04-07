import matplotlib.pyplot as plt
from matplotlib.lines import Line2D


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


def main():
    sqr_file = 'outputs/sqr_output.txt'
    mul_file = 'outputs/mul_output.txt'
    mod_file = 'outputs/modulo_output.txt'

    threshold = 100
    plot_start = 0  # change these
    plot_end = plot_start + 100  # change these

    sqr = parse_file(sqr_file)
    mul = parse_file(mul_file)
    mod = parse_file(mod_file)

    plot_slots = list(range(plot_start, plot_end))

    fig, ax = plt.subplots(figsize=(16, 6))

    for s in plot_slots:
        if s in sqr:
            t = sqr[s]
            ax.scatter(s, t, color='red' if t < threshold else '#ffaaaa', s=15, zorder=3)
        if s in mul:
            t = mul[s]
            color = 'black' if t < threshold else '#cccccc'
            fc = 'black' if t < threshold else 'none'
            ax.scatter(s, t, color=color, facecolors=fc, s=15, marker='o', zorder=3)
        if s in mod:
            t = mod[s]
            ax.scatter(s, t, color='blue' if t < threshold else '#aaaaff', s=15, zorder=3)

    ax.axhline(y=threshold, color='black', linestyle='--', linewidth=1)
    ax.text(plot_start + 2, threshold + 5, 'Threshold', fontsize=9)

    legend_elements = [
        Line2D([0], [0], marker='o', color='w', markerfacecolor='red', markersize=8, label='Square'),
        Line2D([0], [0], marker='o', color='w', markerfacecolor='black', markersize=8, label='Multiply'),
        Line2D([0], [0], marker='o', color='w', markerfacecolor='blue', markersize=8, label='Modulo'),
    ]
    ax.legend(handles=legend_elements, loc='upper right')
    ax.set_xlabel('Time Slot Number')
    ax.set_ylabel('Probe Time (cycles)')
    ax.set_title(f'Flush+Reload Spy Output (slots {plot_start}-{plot_end})')
    ax.set_ylim(0, 500)

    plt.tight_layout()
    plt.savefig('outputs/spy_output.png', dpi=150)
    print("Saved to outputs/spy_output.png")
    plt.show()


if __name__ == '__main__':
    main()