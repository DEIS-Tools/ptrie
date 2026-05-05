#!/usr/bin/env python3
"""Script to visualize google-benchmark output"""
from __future__ import print_function
import argparse
import math
import logging
import json
import pandas as pd
import matplotlib.pyplot as plt
import pathlib

logging.basicConfig(format="[%(levelname)s] %(message)s")

METRICS = [
    "real_time",
    "cpu_time",
    "bytes_per_second",
    "items_per_second",
    "iterations",
]
TRANSFORMS = {"": lambda x: x, "inverse": lambda x: 1.0 / x}

fig, ax = plt.subplots(layout="constrained")
map_legend_to_ax = {}  # Will map legend lines to original lines.

def get_default_ylabel(args):
    """Compute default ylabel for commandline args"""
    label = ""
    if args.transform == "":
        label = args.metric
    else:
        label = args.transform + "(" + args.metric + ")"
    if args.relative_to is not None:
        label += " relative to %s" % args.relative_to
    return label


def parse_args():
    """Parse commandline arguments"""
    parser = argparse.ArgumentParser(description="Visualize google-benchmark output")
    parser.add_argument(
        "-m",
        metavar="METRIC",
        choices=METRICS,
        default=METRICS[0],
        dest="metric",
        help="metric to plot on the y-axis, valid choices are: %s" % ", ".join(METRICS),
    )
    parser.add_argument(
        "-t",
        metavar="TRANSFORM",
        choices=TRANSFORMS.keys(),
        default="",
        help="transform to apply to the chosen metric, valid choices are: %s"
             % ", ".join(list(TRANSFORMS)),
        dest="transform",
    )
    parser.add_argument(
        "-r",
        metavar="RELATIVE_TO",
        type=str,
        default=None,
        dest="relative_to",
        help="plot metrics relative to this label",
    )
    parser.add_argument(
        "--xlabel", type=str, default="input size", help="label of the x-axis"
    )
    parser.add_argument("--ylabel", type=str, help="label of the y-axis")
    parser.add_argument("--title", type=str, default="", help="title of the plot")
    parser.add_argument(
        "--logx", action="store_true", help="plot x-axis on a logarithmic scale"
    )
    parser.add_argument(
        "--log2x", action="store_true", help="plot x-axis on a logarithmic base 2 scale"
    )
    parser.add_argument(
        "--logy", action="store_true", help="plot y-axis on a logarithmic scale"
    )
    parser.add_argument(
        "--log2y", action="store_true", help="plot y-axis on a logarithmic base 2 scale"
    )
    parser.add_argument(
        "--output", type=str, default="", help="File in which to save the graph"
    )
    parser.add_argument('filepath', nargs='*')

    args = parser.parse_args()

    if args.ylabel is None:
        args.ylabel = get_default_ylabel(args)
    return args


def parse_input_size(name):
    parts = name.split("/")
    if len(parts) == 1:
        return 1
    if parts[1].startswith("threads:"):
        return int(parts[1].split(":")[1])
    return int(parts[1])


def read_data(args):
    """Read and process dataframe using commandline args"""
    all_data = []
    for file_path in args.filepath:
        fpath = pathlib.Path(file_path)
        try:
            if fpath.suffix == ".csv":
                data = pd.read_csv(fpath, usecols=["name", args.metric])
            elif fpath.suffix == ".json":
                json_data = json.load(open(fpath,'r'))
                data = pd.DataFrame(json_data["benchmarks"])
            else:
                logging.error("Unsupported file extension '{}'".format(fpath.suffix))
                exit(1)
        except ValueError:
            logging.error(
                f'Could not parse {fpath}. Did you forget "--benchmark_format=[csv|json] when running the benchmark"?'
            )
            exit(1)
        data["label"] = data["name"].apply(lambda x: x.split("/")[0])
        data["input"] = data["name"].apply(parse_input_size)
        data[args.metric] = data[args.metric].apply(TRANSFORMS[args.transform])
        all_data.append(data)
    return pd.concat(all_data)

def plot_groups(label_groups, args):
    """Display the processed data"""
    fig.canvas.manager.set_window_title(args.title)
    ax.set_title(args.title)
    ax.set_xlabel(args.xlabel)
    ax.set_ylabel(args.ylabel)
    ax.xaxis.grid()
    ax.yaxis.grid()
    ax_lines = []
    for label, group in label_groups.items():
        (line, ) = ax.plot(group["input"], group[args.metric], label=label, marker=".")
        ax_lines.append(line)
    leg = ax.legend(loc="center left", bbox_to_anchor=(1, 0.5), ncol=1);
    pickradius = 5  # Points (Pt). How close the click needs to be to trigger an event.
    for legend_line, ax_line in zip(leg.get_lines(), ax_lines):
        legend_line.set_picker(pickradius)  # Enable picking on the legend line.
        map_legend_to_ax[legend_line] = ax_line
    if args.logx:
        plt.xscale("log")
    if args.log2x:
        plt.xscale("log", base=2)
    if args.logy:
        plt.yscale("log")
    if args.log2y:
        plt.yscale("log", base=2)
    if args.output:
        logging.info("Saving to %s" % args.output)
        plt.savefig(args.output)
    else:
        fig.canvas.mpl_connect('pick_event', on_pick)
        fig.canvas.mpl_connect('button_press_event', on_press)
        leg.set_draggable(True) # allow to drag the leggend
        plt.show()

def on_pick(event):
    # https://matplotlib.org/stable/gallery/event_handling/legend_picking.html
    # On the pick event, find the original line corresponding to the legend
    # proxy line, and toggle its visibility.
    legend_line = event.artist

    # Do nothing if the source of the event is not a legend line.
    if legend_line not in map_legend_to_ax:
        return

    ax_line = map_legend_to_ax[legend_line]
    visible = not ax_line.get_visible()
    ax_line.set_visible(visible)
    legend_line.set_alpha(1.0 if visible else 0.2)
    fig.canvas.draw()

def on_press(event):
    if event.button == 3:
        if event.dblclick:
            for (legend_line, ax_line) in map_legend_to_ax.items():
                ax_line.set_visible(False)
                legend_line.set_alpha(0.2)
        else:
            for (legend_line, ax_line) in map_legend_to_ax.items():
                visible = not ax_line.get_visible()
                ax_line.set_visible(visible)
                legend_line.set_alpha(1.0 if visible else 0.2)
        fig.canvas.draw()

def main():
    """Entry point of the program"""
    args = parse_args()
    data = read_data(args)
    label_groups = {}
    for label, group in data.groupby("label"):
        label_groups[label] = group.set_index("input", drop=False)
    if args.relative_to is not None:
        try:
            baseline = label_groups[args.relative_to][args.metric].copy()
        except KeyError as key:
            msg = "Key %s is not present in the benchmark output"
            logging.error(msg, str(key))
            exit(1)

    if args.relative_to is not None:
        for label in label_groups:
            label_groups[label][args.metric] /= baseline
    plot_groups(label_groups, args)


if __name__ == "__main__":
    main()
