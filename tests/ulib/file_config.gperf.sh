#!/bin/sh

# need to remove inline and unsigned int into uint32_t
gperf file_config.key --language=C --output-file file_config.gperf
