import pandas as pd
import glob
import os
from os import path


def transpose_csv(path, outpath):
    pd.read_csv(path, header=None).T.to_csv(outpath, header=False, index=False)


def scale_csv(path, output):

    df = pd.read_csv(path)

    col_names = ['DD', 'DP', 'PD', 'PP']
    for col_name in col_names:
        scalor = 1.0
        col = df[col_name]
        for i in range(len(col)):
            col[i] = col[i] * scalor
            scalor *= 2

    df.to_csv(output, index=False)


directory = "./output/2021.04.09.16.24.35"
csv_paths = glob.glob(path.join(directory, "*.csv"))

for csv_path in csv_paths:
    trans_path = csv_path[:-4] + "_T.csv"
    transpose_csv(csv_path, trans_path)
    scale_csv(trans_path, trans_path[:-4] + "_S.csv")
    os.remove(trans_path)
