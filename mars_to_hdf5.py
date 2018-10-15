import uproot
import logging
from fact.io import to_h5py
import numpy as np
import pandas as pd
import click

log = logging.getLogger()
handler = logging.StreamHandler()
formatter = logging.Formatter(
    fmt='[{asctime}] {message}', style='{', datefmt='%Y-%m-%dT%H:%M:%S'
)
handler.setFormatter(formatter)
log.addHandler(handler)
log.setLevel(logging.INFO)

TELESCOPE_COLUMNS = {
    'MHillas_{telescope_id}.fLength': 'length',
    'MHillas_{telescope_id}.fWidth': 'width',
    'MHillas_{telescope_id}.fSize': 'size',
    'MHillas_{telescope_id}.fMeanX': 'cog_x',
    'MHillas_{telescope_id}.fMeanY': 'cog_y',
    'MHillas_{telescope_id}.fDelta': 'delta',
    'MHillasExt_{telescope_id}.fM3Long': 'm3_long',
    'MHillasExt_{telescope_id}.fM3Trans': 'm3_trans',
    'MNewImagePar_{telescope_id}.fLeakage1': 'leakage1',
    'MNewImagePar_{telescope_id}.fLeakage2': 'leakage2',
    'MNewImagePar_{telescope_id}.fConc': 'conc',
    'MImagePar_{telescope_id}.fNumIslands': 'num_islands',
    'MExtraCTAInfo_{telescope_id}.fTelTriggerTime': 'trigger_time',
}

ARRAY_COLUMNS = {
    'MMcEvt_1.fEvtNumber': 'corsika_event_number',
    'MMcEvt_1.fEnergy': 'energy',
    'MMcEvt_1.fTheta': 'theta',
    'MMcEvt_1.fPhi': 'phi',
    'MMcEvt_1.fCoreX': 'core_x',
    'MMcEvt_1.fCoreY': 'core_y',
    'MMcEvt_1.fImpact': 'impact',
    'MMcEvt_1.fTelescopePhi': 'telescope_phi',
    'MMcEvt_1.fTelescopeTheta': 'telescope_theta',
}

@click.command()
@click.argument('inputfile')
@click.argument('outputfile')
def main(inputfile, outputfile):
    logging.info('Opening file')
    f = uproot.open(inputfile)

    logging.info('Getting tree')
    tree = f['Events']
    branches = set(k.decode('ascii') for k in tree.keys())

    ids = np.arange(tree.numentries)
    dfs = []

    logging.info('Start reading telescope events')
    telescope_id = 1
    while f'MHillas_{telescope_id}.' in branches:
        columns = {
            k.format(telescope_id=telescope_id): v
            for k, v in TELESCOPE_COLUMNS.items()
        }

        df = tree.pandas.df(columns.keys())
        df.rename(columns=columns, inplace=True)
        df['event_id'] = ids
        df['telescope_id'] = telescope_id
        dfs.append(df)
        telescope_id += 1

    df = pd.concat(dfs)
    df = df[df.trigger_time != -100]

    logging.info(f'Writing {len(df)} telescope events to hdf5 file')
    to_h5py(df, outputfile, mode='w', key='telescope_events')
    logging.info('done')

    df = tree.pandas.df(ARRAY_COLUMNS.keys())
    df.rename(columns=ARRAY_COLUMNS, inplace=True)
    df['event_id'] = ids
    logging.info(f'Writing {len(df)} array events to hdf5 file')
    to_h5py(df, outputfile, mode='a', key='array_events')
    logging.info('done')


if __name__ == '__main__':
    main()
