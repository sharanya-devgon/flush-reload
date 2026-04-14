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
    plot_start = 100
    plot_end = plot_start + 100

    sqr = parse_file(sqr_file)
    mul = parse_file(mul_file)
    mod = parse_file(mod_file)

    plot_slots = list(range(plot_start, plot_end))

    fig, ax = plt.subplots(figsize=(16, 6))

    for s in plot_slots:
        # Square: large red filled circle (hits and misses)
        if s in sqr:
            t = sqr[s]
            ax.scatter(s, t, color='red' if t < threshold else '#ffaaaa',
                       s=40, zorder=3)

        # Modulo: small blue filled circle (hits and misses)
        if s in mod:
            t = mod[s]
            ax.scatter(s, t, color='blue' if t < threshold else '#aaaaff',
                       s=12, zorder=4)

        # Multiply: always plot misses as grey hollow, HITs as black hollow ring
        if s in mul:
            t = mul[s]
            if t < threshold:
                ax.scatter(s, t, facecolors='none', edgecolors='black',
                           linewidths=1.5, s=80, zorder=5)
            else:
                ax.scatter(s, t, facecolors='none', edgecolors='#aaaaaa',
                           linewidths=0.8, s=40, zorder=2)

    ax.axhline(y=threshold, color='black', linestyle='--', linewidth=1)
    ax.text(plot_start + 2, threshold + 5, 'Threshold', fontsize=9)

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