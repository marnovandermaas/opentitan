# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
r"""
Class describing simulation configuration object
"""

import collections
import fnmatch
import logging as log
import os
import shutil
import subprocess
import sys
from collections import OrderedDict
from pathlib import Path

from Deploy import CompileOneShot
from FlowCfg import FlowCfg
from OneShotCfg import OneShotCfg
from Modes import BuildModes, Modes, Regressions, RunModes, Tests
from SimResults import SimResults
from tabulate import tabulate
from Testplan import Testplan
from utils import VERBOSE, rm_path


class CosimCfg(FlowCfg):
    """Ibex cosimulation configuration object

    A cosimulation configuration class holds key information required for
    building a DV regression framework.
    """

    ignored_wildcards = (FlowCfg.ignored_wildcards +
                         ['build_mode', 'index', 'test'])

    flow = 'cosim'

    def __init__(self, flow_cfg_file, hjson_data, args, mk_config):
        print("CosimCfg.py: Placeholder __init__")

        # Objects to be populated
        self.build_opts = []
        self.deploy = []
        self.build_list = []
        self.links = {}

        # Necessary for build
        self.fail_patterns = []
        self.report_cmd = ""
        self.report_opts = []
        self.flow_makefile = ""
        self.dry_run = args.dry_run

        super().__init__(flow_cfg_file, hjson_data, args, mk_config)

    def _expand(self):
        super()._expand()
        print("CosimCfg.py: Placeholder _expand")

        # Set directories with links for ease of debug / triage.
        self.links = {
            "D": self.scratch_path + "/" + "dispatched",
            "P": self.scratch_path + "/" + "passed",
            "F": self.scratch_path + "/" + "failed",
            "K": self.scratch_path + "/" + "killed"
        }

        if not hasattr(self, "build_mode"):
            setattr(self, "build_mode", "default")

        # Create objects from raw dicts - build_modes, sim_modes, run_modes,
        # tests and regressions, only if not a primary cfg obj
        self._create_objects()

    # Purge the output directories. This operates on self.
    def _purge(self):
        print("CosimCfg.py: Placeholder _purge")

    def _create_objects(self):
        # Create build and run modes objects
        build_modes = Modes.create_modes(BuildModes,
                                         getattr(self, "build_modes"))
        setattr(self, "build_modes", build_modes)
        print(build_modes)

        # All defined build modes are being built, hence extend all with the
        # global opts.
        for build_mode in build_modes:
            build_mode.build_opts.extend(self.build_opts)

    def _print_list(self):
        print("CosimCfg.py: Placeholder _print_list")

    def _create_dirs(self):
        '''Create initial set of directories
        '''
        for link in self.links.keys():
            rm_path(self.links[link])
            os.makedirs(self.links[link])

    def _create_deploy_objects(self):
        '''Create deploy objects from the build and run lists.
        '''
        print("CosimCfg.py: Placeholder _create_deploy_objects")
        builds = []
        build_map = {}
        for build in self.build_modes:
            item = CompileOneShot(build, self)
            builds.append(item)
            build_map[build] = item

        self.deploy = builds

        # Create initial set of directories before kicking off the regression.
        self._create_dirs()

    def _gen_results(self, run_results):
        '''
        The function is called after the regression has completed. It collates the
        status of all run targets and generates a dict. It parses the testplan and
        maps the generated result to the testplan entries to generate a final table
        (list). It also prints the full list of failures for debug / triage. If cov
        is enabled, then the summary coverage report is also generated. The final
        result is in markdown format.
        '''

        # Generate results table for runs.
        results_str = "## Placeholder Cosim results\n"
        return results_str

    def gen_results_summary(self):
        '''Generate the summary results table.

        This method is specific to the primary cfg. It summarizes the results
        from each individual cfg in a markdown table.

        Prints the generated summary markdown text to stdout and returns it.
        '''

        lines = [f"## Placeholder Cosim results (Summary)"]
        return lines

    def _publish_results(self):
        '''Publish coverage results to the opentitan web server.'''
        super()._publish_results()
        print("CosimCfg.py: Placeholder _publish_results")

        # if self.cov_report_deploy is not None:
        #     results_server_dir_url = self.results_server_dir.replace(
        #         self.results_server_prefix, self.results_server_url_prefix)

        #     log.info("Publishing coverage results to %s",
        #              results_server_dir_url)
        #     cmd = (self.results_server_cmd + " -m cp -R " +
        #            self.cov_report_dir + " " + self.results_server_dir)
        #     try:
        #         cmd_output = subprocess.run(args=cmd,
        #                                     shell=True,
        #                                     stdout=subprocess.PIPE,
        #                                     stderr=subprocess.STDOUT)
        #         log.log(VERBOSE, cmd_output.stdout.decode("utf-8"))
        #     except Exception as e:
        #         log.error("%s: Failed to publish results:\n\"%s\"", e,
        #                   str(cmd))
