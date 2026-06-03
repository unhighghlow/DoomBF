#![no_std]
#![no_main]

extern crate alloc;
use alloc::vec::Vec;
use core::alloc::Layout;
use core::alloc::GlobalAlloc;
use talc::{*, source::Claim};
use core::arch::asm;
use core::panic::PanicInfo;


struct SyncAlloc {
    inner: TalcCell<Claim>,
}

unsafe impl Send for SyncAlloc {}
unsafe impl Sync for SyncAlloc {}

unsafe impl GlobalAlloc for SyncAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        self.inner.alloc(layout)
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        self.inner.dealloc(ptr, layout)
    }
    // implement other methods as needed
}



#[global_allocator]
static TALC: SyncAlloc = SyncAlloc{inner:TalcCell::new(unsafe {
    Claim::array(core::mem::transmute::<i32, *mut [u8; 0x4000000]>(0x4000000))
})};



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
