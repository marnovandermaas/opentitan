#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
r"""Generate RTL and documentation collateral from OTP memory
map definition file (hjson).
"""
import argparse
import logging as log
import sys
from pathlib import Path
from typing import Dict

from lib.common import wrapped_docstring
from lib.OtpMemMap import OtpMemMap
from mako import exceptions
from mako.template import Template

# This makes topgen libraries available to template files.
sys.path.append(Path(__file__).parents[1])

TABLE_HEADER_COMMENT = '''<!--
DO NOT EDIT THIS FILE DIRECTLY.
It has been generated with ./util/design/gen-otp-mmap.py
-->

'''

TPL_GEN_COMMENT = '''// DO NOT EDIT THIS FILE DIRECTLY.
// It has been generated with ./util/design/gen-otp-mmap.py
'''

# memory map source
MMAP_DEFINITION_FILE = "hw/top_earlgrey/data/otp/otp_ctrl_mmap.hjson"
# documentation tables to generate
PARTITIONS_TABLE_FILE = Path("doc") / "otp_ctrl_partitions.md"
DIGESTS_TABLE_FILE = Path("doc") / "otp_ctrl_digests.md"
MMAP_TABLE_FILE = Path("doc") / "otp_ctrl_mmap.md"
DESC_TABLE_FILE = Path("doc") / "otp_ctrl_field_descriptions.md"

# code templates to render
TEMPLATES_PATH = Path("hw") / "ip_templates" / "otp_ctrl"
DATA_TEMPLATES_PATH = TEMPLATES_PATH / "data"
DV_TEMPLATES_PATH = TEMPLATES_PATH / "dv"
COV_TEMPLATES_PATH = DV_TEMPLATES_PATH / "cov"
ENV_TEMPLATES_PATH = DV_TEMPLATES_PATH / "env"
SEQ_LIB_TEMPLATES_PATH = ENV_TEMPLATES_PATH / "seq_lib"
COV_TEMPLATES = [COV_TEMPLATES_PATH / "otp_ctrl_cov_bind.sv.tpl"]
DATA_TEMPLATES = [DATA_TEMPLATES_PATH / "otp_ctrl.hjson.tpl"]
DIF_TEMPLATES = [
    DATA_TEMPLATES_PATH / "dif_otp_ctrl.c.tpl",
    DATA_TEMPLATES_PATH / "dif_otp_ctrl.h.tpl",
    DATA_TEMPLATES_PATH / "dif_otp_ctrl_unittest.cc.tpl"
]
ENV_TEMPLATES = [
    ENV_TEMPLATES_PATH / "otp_ctrl_env_cov.sv.tpl",
    ENV_TEMPLATES_PATH / "otp_ctrl_env_pkg.sv.tpl",
    ENV_TEMPLATES_PATH / "otp_ctrl_if.sv.tpl",
    ENV_TEMPLATES_PATH / "otp_ctrl_scoreboard.sv.tpl"
]
RTL_TEMPLATES_PATH = TEMPLATES_PATH / "rtl"
RTL_TEMPLATES = [RTL_TEMPLATES_PATH / "otp_ctrl_part_pkg.sv.tpl"]
SEQ_TEMPLATES = [
    SEQ_LIB_TEMPLATES_PATH / "otp_ctrl_base_vseq.sv.tpl",
    SEQ_LIB_TEMPLATES_PATH / "otp_ctrl_dai_lock_vseq.sv.tpl",
    SEQ_LIB_TEMPLATES_PATH / "otp_ctrl_smoke_vseq.sv.tpl"
]


def check_in_repo_top():
    dot_git_path = Path.cwd() / ".git"
    if not dot_git_path.exists():
        log.error('This utility must be run from repo_top')
        exit(1)


def render_template(template_path: Path, target_path: Path,
                    params: Dict[str, object]):
    try:
        tpl = Template(filename=str(template_path))
    except OSError as e:
        log.error(f"Error creating template: {e}")
        exit(1)

    try:
        expansion = tpl.render(**params)
    except exceptions.MakoException:
        log.error(exceptions.text_error_template().render())
        exit(1)

    try:
        with target_path.open(mode='w', encoding='UTF-8') as outfile:
            outfile.write(expansion)
    except OSError as e:
        log.error(f"Error rendering template: {e}")
        exit(1)


def _target_from_template_path(output_path: Path, template: Path) -> Path:
    full_path = output_path / template.relative_to(TEMPLATES_PATH)
    return Path(full_path.parents[0]) / full_path.stem


def main():
    log.basicConfig(level=log.WARNING, format="%(levelname)s: %(message)s")

    parser = argparse.ArgumentParser(
        prog="gen-otp-mmap",
        description=wrapped_docstring(),
        formatter_class=argparse.RawDescriptionHelpFormatter)

    # Generator options for compile time random netlist constants
    parser.add_argument('--seed',
                        type=int,
                        metavar='<seed>',
                        help='Custom seed for RNG to compute default values.')

    parser.add_argument('--topname',
                        required=True,
                        type=str,
                        metavar='<topname>',
                        help='The topname, as in earlgrey or darjeeling.')
    args = parser.parse_args()

    # The placement of sw difs requires this be run from repo_top.
    check_in_repo_top()

    otp_mmap = OtpMemMap.from_mmap_path(
        MMAP_DEFINITION_FILE.replace('earlgrey', args.topname), args.seed)
    partitions = otp_mmap.config["partitions"]
    output_path = (Path("hw") / f"top_{args.topname}" / "ip_autogen" /
                   "otp_ctrl")
    with open(output_path / PARTITIONS_TABLE_FILE, 'wb',
              buffering=2097152) as outfile:
        outfile.write(TABLE_HEADER_COMMENT.encode('utf-8'))
        outfile.write(
            OtpMemMap.create_partitions_table(partitions).encode('utf-8'))
        outfile.write('\n'.encode('utf-8'))

    with open(output_path / DIGESTS_TABLE_FILE, 'wb',
              buffering=2097152) as outfile:
        outfile.write(TABLE_HEADER_COMMENT.encode('utf-8'))
        outfile.write(
            OtpMemMap.create_digests_table(partitions).encode('utf-8'))
        outfile.write('\n'.encode('utf-8'))

    with open(output_path / MMAP_TABLE_FILE, 'wb',
              buffering=2097152) as outfile:
        outfile.write(TABLE_HEADER_COMMENT.encode('utf-8'))
        outfile.write(OtpMemMap.create_mmap_table(partitions).encode('utf-8'))
        outfile.write('\n'.encode('utf-8'))

    with open(output_path / DESC_TABLE_FILE, 'wb',
              buffering=2097152) as outfile:
        outfile.write(TABLE_HEADER_COMMENT.encode('utf-8'))
        outfile.write(
            OtpMemMap.create_description_table(partitions).encode('utf-8'))
        outfile.write('\n'.encode('utf-8'))

    # render all templates
    params = {
        "otp_mmap": otp_mmap.config,
        "gen_comment": TPL_GEN_COMMENT,
        "topname": args.topname
    }
    for template in COV_TEMPLATES:
        target_path = _target_from_template_path(output_path, template)
        render_template(template, target_path, params)
    for template in DATA_TEMPLATES:
        target_path = _target_from_template_path(output_path, template)
        render_template(template, target_path, params)
    # This won't work correctly until top-specific difs are created.
    # Comment it out for now.
    # TODO(lowrisc/opentitan#25743): Enable this once multi-top deals with top-
    # specific difs.
    # for template in DIF_TEMPLATES:
    #     target_path = (Path.cwd() / "sw" / "device" / "lib" / "dif" /
    #                    template.stem)
    #     render_template(template, target_path, params)
    for template in ENV_TEMPLATES:
        target_path = _target_from_template_path(output_path, template)
        render_template(template, target_path, params)
    for template in RTL_TEMPLATES:
        target_path = _target_from_template_path(output_path, template)
        render_template(template, target_path, params)
    for template in SEQ_TEMPLATES:
        target_path = _target_from_template_path(output_path, template)
        render_template(template, target_path, params)


if __name__ == "__main__":
    main()
