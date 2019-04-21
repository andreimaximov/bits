void gotPlt1();

// How does dynamic linking and symbol lookup work on Linux? Answer, using the
// GOT and PLT:
//
// - https://www.technovelty.org/linux/plt-and-got-the-key-to-code-sharing-and-dynamic-libraries.html
// - https://systemoverlord.com/2017/03/19/got-and-plt-for-pwning.html
//
// 1. $ gcc -shared -fPIC -g -O3 -o build/libgot_plt_1.so src/got_plt_1.c
//    $ gcc -shared -fPIC -g -O3 -o build/libgot_plt_2.so src/got_plt_2.c
//    $ gcc -g -O3 -o build/got_plt bin/got_plt.c build/libgot_plt_1.so build/libgot_plt_2.so
//
// 2. Inspect the shared libraries referenced:
//
//    $ ldd build/got_plt
//          linux-vdso.so.1 (0x00007ffef49c8000)
//          build/libgot_plt_1.so (0x00007f010ba77000)
//          build/libgot_plt_2.so (0x00007f010b875000)
//          libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f010b484000)
//          /lib64/ld-linux-x86-64.so.2 (0x00007f010be7b000)
//
//    You should see the address of libc.so (and perhaps others) change with
//    each invocation of ldd.
//
// 3. Let's also inspect relocations:
//
//    $ readelf --relocs build/got_plt
//
//    Relocation section '.rela.plt' at offset 0x5c0 contains 1 entry:
//      Offset          Info           Type           Sym. Value    Sym. Name +
//      Addend
//    000000200fd0  000200000007 R_X86_64_JUMP_SLO 0000000000000000 gotPlt1 + 0
//
//    $ readelf --relocs build/libgot_plt_1.so
//
//    Relocation section '.rela.plt' at offset 0x488 contains 1 entry:
//      Offset          Info           Type           Sym. Value    Sym. Name +
//      Addend
//    000000201018  000b00000007 R_X86_64_JUMP_SLO 0000000000000610 separator + 0
//    000000201020  000400000007 R_X86_64_JUMP_SLO 0000000000000000 gotPlt2 + 0
//
// 4. Finally inspect the gotPlt2 call from gotPlt1:
//
//    $ objdump -D build/libgot_plt_1.so
//
//    Disassembly of section .plt:
//
//    00000000000004f0 <.plt>:
//     4f0:   ff 35 12 0b 20 00       pushq  0x200b12(%rip)        # 201008 <_GLOBAL_OFFSET_TABLE_+0x8>
//     4f6:   ff 25 14 0b 20 00       jmpq   *0x200b14(%rip)       # 201010 <_GLOBAL_OFFSET_TABLE_+0x10>
//     4fc:   0f 1f 40 00             nopl   0x0(%rax)
//
//    0000000000000500 <separator@plt>:
//     500:   ff 25 12 0b 20 00       jmpq   *0x200b12(%rip)        # 201018 <separator+0x200a08>
//     506:   68 00 00 00 00          pushq  $0x0
//     50b:   e9 e0 ff ff ff          jmpq   4f0 <.plt>
//
//    0000000000000510 <gotPlt2@plt>:
//     510:   ff 25 0a 0b 20 00       jmpq   *0x200b0a(%rip)        # 201020 <gotPlt2>
//     516:   68 01 00 00 00          pushq  $0x1
//     51b:   e9 d0 ff ff ff          jmpq   4f0 <.plt>
//
//    ...
//
//    Disassembly of section .text:
//
//     0000000000000620 <gotPlt1>:
//      620:   48 83 ec 08             sub    $0x8,%rsp
//      624:   31 c0                   xor    %eax,%eax
//      626:   e8 e5 fe ff ff          callq  510 <gotPlt2@plt>
//      62b:   31 c0                   xor    %eax,%eax
//      62d:   e8 ce fe ff ff          callq  500 <separator@plt>
//      632:   31 c0                   xor    %eax,%eax
//      634:   48 83 c4 08             add    $0x8,%rsp
//      638:   e9 d3 fe ff ff          jmpq   510 <gotPlt2@plt>
//
///   ...
//
//    Disassembly of section .got.plt:
//
//    0000000000201000 <_GLOBAL_OFFSET_TABLE_>:
//      201000:       60                      (bad)
//      201001:       0e                      (bad)
//      201002:       20 00                   and    %al,(%rax)
//            ...
//      201018:       06                      (bad)
//      201019:       05 00 00 00 00          add    $0x0,%eax
//      20101e:       00 00                   add    %al,(%rax)
//      201020:       16                      (bad)
//      201021:       05 00 00 00 00          add    $0x0,%eax
//            ...
//
//     You can read the included "PLT and GOT - the key to code sharing and
//     dynamic libraries" article to understand what is going on here.
//
// 5. What happens at runtime?
//
//    $ gdb build/got_plt
//    (gdb) b gotPlt1
//    (gdb) b separator
//    (gdb) r
//    (gdb) layout asm
//
//    We should see assembly like this for the calls to gotPlt2 from gotPlt1:
//
//    0x7ffff7bd3626 <gotPlt1+6>  callq  0x7ffff7bd3510 <gotPlt2@plt>
//
// 6. (gdb) si
//
//    Run this a couple of times until you are in gotPlt2@plt. You should see
//    some assembly that looks like this:
//
//    0x7ffff7bd3510 <gotPlt2@plt>     jmpq   *0x200b0a(%rip)        # 0x7ffff7dd4020
//    0x7ffff7bd3516 <gotPlt2@plt+6>   pushq  $0x1
//    0x7ffff7bd351b <gotPlt2@plt+11>  jmpq   0x7ffff7bd34f0
//
// 7. We can inspect the jump address:
//
//     (gdb) x 0x7ffff7dd4020
//     0x7ffff7dd4020: 0xf7bd3516
//
//     We are on instruction 0x7ffff7bd3510 so we should expect to just jump to
//     the next instruction at 0xf7bd3516.
//
// 8. Continue (you can si slowly if you want to see all the dynamic loader
//    instructions) until the call to separator and inspect the previous jump
//    address:
//
//    (gdb) c
//    (gdb) x 0x7ffff7dd4020
//    0x7ffff7dd4020: 0xf79d1580
//
//    Whoa! The jump address changed! I wonder what this address points to?
//
// 9. (gdb) si
//
//    Run this a couple times until we are on the trampoline to call gotPlt2 again. Notice we now jump directly to gotPlt2.
//
//    (gdb) x $pc
//    0x7ffff79d1580 <gotPlt2>:       0x0000c3f3
//
//    Ah! So the dynamic loader cached the instruction address of gotPlt2 in the
//    GOT.
int main(int argc, char** argv) {
  gotPlt1();
  return 0;
}
