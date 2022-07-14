sudo qemu-system-x86_64  -cpu host -enable-kvm -m 128 -nodefaults  -display none -serial stdio -kernel build/app-helloworld2_kvm-x86_64.dbg
