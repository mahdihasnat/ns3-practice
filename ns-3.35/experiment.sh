# Add bash shebang
#!/bin/bash
# Path: ns-3.35/experiment.sh
cd ns-3.35
./waf -j8 --run "scratch/manet.cc --n=2 --nFlows=2"