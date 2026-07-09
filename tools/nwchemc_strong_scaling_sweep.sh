#!/usr/bin/env bash
# Fixed-problem strong-scaling sweep for nwchemc_mpi_force_timer.
# Usage: NWCHEMC_LIBRARY=.../libnwchemc.so ./nwchemc_strong_scaling_sweep.sh \
#          params.bin out.tsv [ranks...]
set -euo pipefail
params=${1:?params.bin}
out=${2:?out.tsv}
shift 2
ranks=("$@")
if [[ ${#ranks[@]} -eq 0 ]]; then
  ranks=(1 2 4)
fi
timer=${NWCHEMC_MPI_FORCE_TIMER:-./nwchemc_mpi_force_timer}
lib=${NWCHEMC_LIBRARY:-libnwchemc.so}
echo -e "rank\twall_s\tenergy_h\tmaxabs_g\tok\tS_P" >"$out"
t1=""
for p in "${ranks[@]}"; do
  line=$(mpirun -np "$p" --bind-to none \
    env OMPI_MCA_prte_create_session_dirs=true \
        OMPI_MCA_pml=ob1 OMPI_MCA_btl=self,tcp \
        LD_LIBRARY_PATH="$(dirname "$lib"):${LD_LIBRARY_PATH:-}" \
    "$timer" "$params" "$lib" 2>&1 | tee /dev/stderr | grep '^nwchemc_mpi_force' | tail -1)
  wall=$(echo "$line" | sed -n 's/.*wall_s=\([^ ]*\).*/\1/p')
  e=$(echo "$line" | sed -n 's/.*energy_h=\([^ ]*\).*/\1/p')
  g=$(echo "$line" | sed -n 's/.*maxabs_g=\([^ ]*\).*/\1/p')
  ok=$(echo "$line" | sed -n 's/.*ok=\([^ ]*\).*/\1/p')
  if [[ -z "$t1" ]]; then t1=$wall; fi
  s=$(python3 -c "print(float('$t1')/float('$wall') if float('$wall')>0 else float('nan'))")
  echo -e "$p\t$wall\t$e\t$g\t$ok\t$s" >>"$out"
  echo "P=$p wall=$wall S=$s ok=$ok"
done
