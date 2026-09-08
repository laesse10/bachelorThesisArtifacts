#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --time=00:04:00
#SBATCH --output=pathtest.%j.out
echo "SBATCH-LEVEL PATH=$PATH"
echo "sbatch which g++: $(which g++)"; g++ --version|head -1
echo "--- under srun ---"
srun --exclusive bash -c 'echo "srun PATH=$PATH"; echo "srun which g++: $(which g++)"; g++ --version|head -1'
echo "--- under srun+perf+python ---"
srun --exclusive perf stat -x, -e cycles -o /dev/null /capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python -c "
import shutil,subprocess,os
print('py PATH=',os.environ.get('PATH','')[:120])
print('py which g++:',shutil.which('g++'))
print(subprocess.run(['g++','--version'],capture_output=True,text=True).stdout.splitlines()[0])"
