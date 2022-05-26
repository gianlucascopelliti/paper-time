## PoC to move in-enclave TSC backwards

```
[sched.c] continuing on CPU 1
[main.c] Creating enclave...

--------------------------------------------------------------------------------
[main.c] calling enclave with do_attack=0
--------------------------------------------------------------------------------

[main.c] hi from test_ocall; do_attack=0

[main.c] enclave measured TSC1=1870449796095838; TSC2=1870449796113289; diff=17451


--------------------------------------------------------------------------------
[main.c] calling enclave with do_attack=1
--------------------------------------------------------------------------------

[main.c] hi from test_ocall; do_attack=1

[file.c] writing buffer to '/dev/cpu/1/msr' (size=8)
[main.c] enclave measured TSC1=1870449796146447; TSC2=6397; diff=18444873623913411566

[main.c] all is well; exiting..
```
