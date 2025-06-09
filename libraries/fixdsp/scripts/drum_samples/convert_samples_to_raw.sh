#!/bin/bash

set -e

waveform_picture () {
    ffmpeg -i "$1" -filter_complex "showwavespic=s=640x320:colors=black:split_channels=1" -frames:v 1 "$2"
}

rates='8000 11025 16000 22050 32000 44100 48000 96000'

for sample_rate in ${rates}; do
    for src_path in `find . -name "*.wav" -type f`; do
        rm -f normalized.wav resample.tmp

        echo "========= ${src_path} =========="
        src_path=${PWD}/${src_path}
        src_file="$(basename ${src_path})"
        src_stem="${src_file%.*}"
        src_dir="$(dirname ${src_path})"
        raw_dir="${src_dir}/raw_8bit_${sample_rate}"
        raw_path="${raw_dir}/${src_stem}.raw"
        raw_half_rate_path="${raw_dir}/${src_stem}_half_rate.raw"
        waveform_dir="${src_dir}/waveform_${sample_rate}"
        waveform_in_path="${waveform_dir}/${src_stem}-input.png"
        waveform_norm_path="${waveform_dir}/${src_stem}-norm.png"
        
        mkdir -p "${raw_dir}"
        ffmpeg -y -i "${src_path}" -ac 1 -ar ${sample_rate} -f s8 -acodec pcm_s8 "resample.tmp"
        ffmpeg -y -i "${src_path}" -ac 1 -ar $((${sample_rate} / 2)) -f s8 -acodec pcm_s8 "resample_half_rate.tmp"
        
        # This is the only solution I found to have raw samples without the WAV
        # header...
        echo "write raw sample at ${raw_path}"
        dd bs=44 skip=1 if="resample.tmp" of="${raw_path}"
        dd bs=44 skip=1 if="resample_half_rate.tmp" of="${raw_half_rate_path}"

        ada_dir=${PWD}/../../src/samples_SR${sample_rate}
        mkdir -p ${ada_dir}

        # Convert to Ada specs
        ./raw_s8_to_ada_specs.py ${raw_path} ${sample_rate}
        ./raw_s8_to_ada_specs.py ${raw_half_rate_path} ${sample_rate}

    done
done
