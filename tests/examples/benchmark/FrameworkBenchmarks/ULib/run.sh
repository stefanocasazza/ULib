#/bin/sh

export FWROOT=/mnt/data/FrameworkBenchmarks      && \
export TROOT=/mnt/data/FrameworkBenchmarks/frameworks/C++/ULib       && \
export IROOT=/mnt/data/FrameworkBenchmarks/installs       && \
export DBHOST=127.0.0.1      && \
export LOGDIR=/mnt/data/FrameworkBenchmarks/results/ec2/latest/logs/ulib      && \
export MAX_THREADS=4 && \
cd /mnt/data/FrameworkBenchmarks/frameworks/C++/ULib && \
sudo -u stefano -E -H stdbuf -o0 -e0 bash -ex setup_plaintext.sh
