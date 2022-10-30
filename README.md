# README
This repo contains 3 test for PRs #564[https://github.com/unikraft/unikraft/pull/564], #565[https://github.com/unikraft/unikraft/pull/565], #566[https://github.com/unikraft/unikraft/pull/566].
Tests `test_pr_564.c` and `test_pr_565.c` don't require Musl to build.
Test `test_pr_565.c` requires enabling support for userland TLS from menuconfig (syscall shim -> binary system call -> support for userland TLS).
Test `test_pr_566.c` requires musl, this branch to be specific: [https://github.com/dragosargint/lib-musl/tree/thread_support].
To run with the same setup as I did:
Use CONFIG_TO__BE_COPIED for .config (but modify the paths) and run `runquemu.sh`
