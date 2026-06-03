#![no_std]
#![no_main]

extern crate alloc;
use alloc::vec::Vec;
use core::alloc::Layout;
use core::alloc::GlobalAlloc;
use core::arch::asm;
use core::panic::PanicInfo;


use linked_list_allocator::LockedHeap;

#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();



#[no_mangle]
pub extern "C" fn _start() -> ! {
    let c: char = 'a';
    put_char(c);

    let mut vec = Vec::with_capacity(100);

    put_char(c);
    vec.extend(0..300usize);

    put_char(c);

    loop {}
}

fn put_char(c: char) -> () {
    let mut a0: u32 = 1;
    let a1: *const char = &c;
    let size: u32 = 1;

    unsafe {
        asm!(
            "ecall",
            inout("a0") a0,
            in("a1") a1,
            in("a2") size,
            in("a7") 64,
        );
    };
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
