import pandas as pd
import glob
import os
from os import path


def scale_pr(directory):

    csv_paths = glob.glob(path.join(directory, "page_rank_*_T.csv"))

    for csv_path in csv_paths:

        df = pd.read_csv(csv_path)

        col_names = ['DD', 'DP', 'PD', 'PP']
        for col_name in col_names:
            scalor = 1.0
            col = df[col_name]
            for i in range(len(col)):
                col[i] = col[i] * scalor
                scalor *= 2

        df.to_csv(csv_path[:-4] + "_S.csv", index=False)


def rename_files(dir):

    csv_paths = glob.glob(path.join(directory, "*.csv"))

    for csv_path in csv_paths:
        os.rename(csv_path, csv_path[:-4] + "_cores.csv")


def transpose_pr(dir):
    csv_paths = glob.glob(path.join(directory, "page_rank_*.csv"))

    for csv_path in csv_paths:
        pd.read_csv(csv_path, header=None).T.to_csv(csv_path[:-4] + "_T.csv", header=False, index=False)


directory = "./output/2021.04.24.12.40.54"

transpose_pr(directory)
scale_pr(directory)
