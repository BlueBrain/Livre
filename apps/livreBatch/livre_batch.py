#!/usr/bin/env python
"""
Usage: livre_batch.py --config file.config

Launch livre in batch mode using sbatch to render images on cluster nodes.
"""

import argparse
import glob
import json
import math
import os
import subprocess

__author__ = "Daniel Nachbaur"
__email__ = "daniel.nachbaur@epfl.ch"
__copyright__ = "Copyright 2015, EPFL/Blue Brain Project"

# pylint: disable=W0142

SECTION_SLURM = 'slurm'
SLURM_NAME = 'job_name'
SLURM_TIME = 'job_time'
SLURM_QUEUE = 'queue'
SLURM_ACCOUNT = 'account'
SLURM_OUTPUTDIR = 'output_dir'
SLURM_NODES = 'nodes'
SLURM_TASKS_PER_NODE = 'tasks_per_node'

SECTION_LIVRE = 'livre'
LIVRE_CAMERA_LOOKAT = 'camera_lookat'
LIVRE_CAMERA_POSITION = 'camera_position'
LIVRE_ENDFRAME = 'end_frame'
LIVRE_HEIGHT = 'height'
LIVRE_LAYOUT = 'eq_layout'
LIVRE_MAXFRAMES = 'max_frames'
LIVRE_SAMPLES_PER_RAY = 'samples_per_ray'
LIVRE_SSE = 'sse'
LIVRE_STARTFRAME = 'start_frame'
LIVRE_TRANSFER_FUNCTION = 'transfer_function'
LIVRE_VOLUME = 'volume'
LIVRE_WIDTH = 'width'

EXAMPLE_JSON = 'example.json'


def find_livre():
    """
    Search for livre executable in PATH and return result
    """

    from distutils import spawn
    livre_path = spawn.find_executable("livre")
    if not livre_path:
        print("Cannot find livre executable in PATH")
        return False
    print("Using livre executable '{0}'".format(livre_path))
    return True

class LivreBatch(object):
    """
    Submits sbatch jobs performing rendering using Livre by using a
    configuration file for setup.
    """

    def __init__(self, verbose, dry_run):
        self.verbose = verbose
        self.dry_run = dry_run
        self.dict = {}
        self.default_dict = {}
        self._fill_default_dict()

    def _fill_default_dict(self):
        """
        Setup default values for all supported options in the configuration file
        """

        self.default_dict = {
            SECTION_SLURM: {
                SLURM_NAME: 'livre_batch',
                SLURM_TIME: '06:00:00',
                SLURM_QUEUE: 'prod',
                SLURM_ACCOUNT: 'proj3',
                SLURM_OUTPUTDIR: '.',
                SLURM_NODES: 1,
                SLURM_TASKS_PER_NODE: 16},
            SECTION_LIVRE: {
                LIVRE_CAMERA_LOOKAT: '0 0 0',
                LIVRE_CAMERA_POSITION: '0 0 1',
                LIVRE_ENDFRAME: 100,
                LIVRE_HEIGHT: 1200,
                LIVRE_LAYOUT: 'Simple',
                LIVRE_MAXFRAMES: 50,
                LIVRE_SAMPLES_PER_RAY: 2048,
                LIVRE_SSE: 1,
                LIVRE_STARTFRAME: 0,
                LIVRE_TRANSFER_FUNCTION: '',
                LIVRE_VOLUME: '',
                LIVRE_WIDTH: 1920}}

    def _build_sbatch_script(self, start, end):
        """
        Build sbatch script for a certain frame range
        """

        values = self.dict
        values['start'] = start
        values['end'] = end
        values['num_frames'] = end - start

        sbatch_script = '\n'.join((
            "#!/bin/bash",
            "#SBATCH --job-name=\"{slurm[job_name]}\"",
            "#SBATCH --time={slurm[job_time]}",
            "#SBATCH --partition={slurm[queue]}",
            "#SBATCH --account={slurm[account]}",
            "#SBATCH --nodes={slurm[nodes]}",
            "#SBATCH --ntasks-per-node={slurm[tasks_per_node]}",
            "#SBATCH --output={slurm[output_dir]}/%j_out.txt",
            "#SBATCH --error={slurm[output_dir]}/%j_err.txt",
            "",
            "export EQ_WINDOW_IATTR_HINT_WIDTH={livre[width]}",
            "export EQ_WINDOW_IATTR_HINT_HEIGHT={livre[height]}",
            "export EQ_CHANNEL_SATTR_DUMP_IMAGE={image}",
            "export ZEROEQ_SESSION=$SLURM_JOB_ID",
            "livre --eq-layout {livre[eq_layout]} --volume {livre[volume]} "\
            "--sse {livre[sse]} --samples-per-ray {livre[samples_per_ray]} "\
            "--synchronous --animation --frames \"{start} {end}\" "\
            "--num-frames {num_frames} "\
            "--camera-position \"{livre[camera_position]}\" "\
            "--camera-lookat \"{livre[camera_lookat]}\" "\
            "--transfer-function \"{livre[transfer_function]}\""
        )).format(**values)

        if self.verbose:
            print(sbatch_script)
        return sbatch_script

    def write_example_config(self):
        """
        Write example configuration to current directory
        """

        with open(EXAMPLE_JSON, 'w') as configfile:
            json.dump(self.default_dict, configfile, sort_keys=True, indent=4,
                      ensure_ascii=False)
        print("Wrote {0} to current directory".format(EXAMPLE_JSON))

    def read_config(self, config):
        """
        Read configuration file and validate content
        """

        with open(config) as configfile:
            self.dict = json.loads(configfile.read())

        volume = self.dict.get(SECTION_LIVRE).get(LIVRE_VOLUME, '')
        if not volume:
            print("Error: Need valid volume URI")
            return False

        self.dict['image'] = "{slurm[output_dir]}/{slurm[job_name]}_".format(**self.dict)

        return True

    def submit_jobs(self):
        """
        Submit jobs from frame range specified in configuration, but checks
        for existing frames/images in output directory to submit jobs only for
        missing frames.
        """

        livre_dict = self.dict[SECTION_LIVRE]
        start_frame = livre_dict[LIVRE_STARTFRAME]
        end_frame = livre_dict[LIVRE_ENDFRAME]

        outdir = self.dict[SECTION_SLURM][SLURM_OUTPUTDIR]
        if not os.path.exists(outdir):
            os.makedirs(outdir)

        # look for already rendered frames
        img_prefix = self.dict['image']
        files = glob.glob('{0}*.png'.format(img_prefix))
        found_frames = set(int(x[len(img_prefix):-4]) for x in files)

        if not found_frames:
            ranges = [(start_frame, end_frame)]
        else:
            # find missing frames
            ideal_range = set(range(start_frame, end_frame))
            missing_frames = list(ideal_range - found_frames)
            missing_frames.sort()

            if not missing_frames:
                print("No missing frames found, no jobs will be submitted.")
                return
            print("Found {0} missing frames".format(len(missing_frames)))

            def _calc_ranges(i):
                """
                http://stackoverflow.com/questions/4628333/converting-a-list-of-integers-into-range-in-python
                """
                from itertools import groupby
                for a, b in groupby(enumerate(i), lambda xy: (xy[1] - xy[0])):
                    b = list(b)
                    yield b[0][1], b[-1][1]
            ranges = _calc_ranges(missing_frames)

        # Submit job(s) for range(s)
        idx = 1
        for sub_range in ranges:
            idx += self._submit_jobs_for_range(idx, sub_range[0],
                                               sub_range[1] + 1)

        if self.dry_run:
            print("{0} job(s) not submitted (dry run)\n".format(idx-1))
        else:
            print("{0} job(s) submitted, find outputs in {1}\n".format(idx-1,
                                                                       outdir))
        return


    def _submit_jobs_for_range(self, idx, start_frame, end_frame):
        """
        Submit batch jobs for a range of frames. Does rebalancing of maximum
        frames per job, according to max frames from configuration and given
        frame range.
        """

        livre_dict = self.dict[SECTION_LIVRE]
        batch_size = livre_dict[LIVRE_MAXFRAMES]
        num_frames = end_frame - start_frame

        num_jobs = int(math.ceil(float(num_frames) / float(batch_size)))
        batch_size = int(math.ceil(float(num_frames) / float(num_jobs)))

        print("Create {0} job(s) with {1} frame(s) each".format(num_jobs,
                                                                batch_size))

        for batch_start in range(start_frame, end_frame, batch_size):
            start = batch_start
            end = min(batch_start + batch_size, end_frame)

            sbatch_script = self._build_sbatch_script(start, end)
            print("Submit job {0} for frames {1} to {2}...".format(idx, start,
                                                                   end))
            idx += 1
            if not self.dry_run:
                sbatch = subprocess.Popen(['sbatch'], stdin=subprocess.PIPE)
                sbatch.communicate(input=sbatch_script)

        return num_jobs


def main():
    """
    Entry point for livre batch application does argument parsing and
    calls livre_batch class accordingly.
    """

    parser = argparse.ArgumentParser(description="Submit sbatch job(s) \
                                              launching Livre to render images")
    parser.add_argument("-c", "--config", help="path to config file")
    parser.add_argument("--dry-run", action="store_true",
                        help="parse config file, but do not submit any jobs")
    parser.add_argument("-e", "--example-config", action="store_true",
                        help="write example.json to current directory")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="print more information")
    args = parser.parse_args()

    livre_batch = LivreBatch(args.verbose, args.dry_run)

    if args.example_config:
        livre_batch.write_example_config()
        exit()

    if not args.config:
        parser.print_help()
        exit()

    if not livre_batch.read_config(args.config):
        exit()

    if not find_livre():
        exit()

    livre_batch.submit_jobs()

if __name__ == "__main__":
    main()
