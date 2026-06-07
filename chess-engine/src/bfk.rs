#![no_std]
#![no_main]

extern crate alloc;
use alloc::vec;
use alloc::vec::Vec;
use core::alloc::Layout;
use core::alloc::GlobalAlloc;
use talc::{*, source::Claim};
use core::arch::asm;
use core::panic::PanicInfo;
use core::convert::TryFrom;



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
}

// Куча в .bss; init_heap() инициализирует talc при старте.
#[used]
static mut HEAP: [u8; 0x400_000] = [0; 0x400_000];

struct BfkAlloc;

unsafe impl GlobalAlloc for BfkAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let talc = core::ptr::addr_of_mut!(TALC);
        unsafe { (*talc).inner.alloc(layout) }
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        let talc = core::ptr::addr_of_mut!(TALC);
        unsafe { (*talc).inner.dealloc(ptr, layout) }
    }
}

static mut TALC: SyncAlloc = SyncAlloc {
    inner: unsafe { TalcCell::new(Claim::new(core::ptr::null_mut(), 0)) },
};

#[global_allocator]
static ALLOC: BfkAlloc = BfkAlloc;

fn init_heap() {
    unsafe {
        TALC.inner = TalcCell::new(Claim::array(core::ptr::addr_of_mut!(HEAP)));
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    print(b"\npanic\n");
    loop {}
}


extern crate chess_engine;
use chess_engine::*;

fn get_cpu_move(b: &Board, best: bool) -> Move {
    let (m, count, _) = if best {
        b.get_best_next_move(4)
    } else {
        b.get_worst_next_move(4)
    };

    print(b"CPU evaluated {} moves before choosing to ");
    print_number_u(count);
    print(b"\n");
    match m {
        Move::Piece(from, to) | Move::Promotion(from, to, _) => {
            match (b.get_piece(from), b.get_piece(to)) {
                (Some(piece), Some(takes)) => {
                    // print(b"take ");
                    // print(takes.get_name());
                    // print(b"(");
                    // print(to);
                    // print(b") with ");
                    // print(piece.get_name());
                    // print(b"(");
                    // print(from);
                    // println(b")");
                },
                (Some(piece), None) => {
                    // print(b"move {}({}) to {}({})");
                    // print(takes.get_name());
                    // print(b"(");
                    // print(from);
                    // print(b") to ");
                    // println(to);
                }
                _ => {
                    // print(b"move ");
                    // print(from);
                    // print(b" ");
                    // println(to);
                },
            }
        }
        Move::KingSideCastle => {
            println(b"castle kingside")
        }
        Move::QueenSideCastle => {
            println(b"castle queenside")
        }
        Move::Resign => println(b"resign"),
    }

    m
}

#[no_mangle]
pub extern "C" fn _start() -> ! {
    init_heap();
    let mut b = Board::default();
    let moves = b.get_legal_moves();
    print_number_u(moves.len() as u32);
    println(b" moves");

    // let mut history: Vec<Move> = vec![];

    // loop {
    //     // let mut s = input(">>> ");
    //     let s: &[u8] = &[];
    //     // s = s.trim().to_string();

    //     let m = if s.is_empty() {
    //         println(b"Waiting for CPU to choose best move...");
    //         get_cpu_move(&b, true)
    //     } else if s == b"worst" {
    //         println(b"Waiting for CPU to choose worst move...");
    //         get_cpu_move(&b, false)
    //     } else if s == b"rate" {
    //         continue;
    //     } else if s == b"pass" {
    //         b = b.change_turn();
    //         continue;
    //     } else if s == b"history" {
    //         for i in 0..history.len() {
    //             if i < history.len() - 1 {
    //                 print_move(history[i]);
    //                 print(b" ");
    //                 print_move(history[i+1]);
    //                 print(b"\n");
    //             } else {
    //                 print_move(history[i]);
    //                 print(b"\n");
    //             }
    //         }
    //         continue;
    //     } else {
    //         match Move::try_from(s) {
    //             Ok(m) => m,
    //             Err(e) => {
    //                 println(b"error");
    //                 continue;
    //             }
    //         }
    //     };

    //     match b.play_move(m) {
    //         GameResult::Continuing(next_board) => {
    //             b = next_board;
    //             print_move(m);
    //             print(b"\n");
    //             history.push(m);
    //         }

    //         GameResult::Victory(winner) => {
    //             // println(b);
    //             // println!("{} loses. {} is victorious.", !winner, winner);
    //             break;
    //         }

    //         GameResult::IllegalMove(x) => {
    //         //     print(x);
    //         //     println(b" is an illegal move.");
    //         }

    //         GameResult::Stalemate => {
    //             println(b"Drawn game.");
    //             break;
    //         }
    //     }
    // }

    // // for m in history {
    // //     println(m);
    // // }

    loop {}
}



fn print(s: &[u8]) -> () {
    let mut a0: u32 = 1;
    let a1: *const u8 = s.as_ptr();
    let size: usize = s.len();

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

fn println(s: &[u8]) -> () {
    print(s);
    put_char(b'\n');
}

fn put_char(c: u8) -> () {
    let s: &[u8; 1] = &[c];
    print(s);
}

fn print_move(mov: Move) -> () {
    match mov {
        Move::Piece(from, to) => {
            print_pos(from);
            print(b" to ");
            print_pos(to);
        },
        Move::Promotion(from, to, piece) => {
            print_pos(from);
            print(b" to ");
            print_pos(to);
            print(b" ");
            print(piece.get_name().as_bytes());  // XXX
        }
        Move::KingSideCastle => print(b"O-O"),
        Move::QueenSideCastle => print(b"O-O-O"),
        Move::Resign => print(b"Resign"),
    }
}

fn print_pos(pos: Position) -> () {
    put_char(
        match pos.col {
            0 => b'a',
            1 => b'b',
            2 => b'c',
            3 => b'd',
            4 => b'e',
            5 => b'f',
            6 => b'g',
            7 => b'h',
            _ => b'?',
        }
    );
    print_number(pos.row + 1);
}

fn print_number(mut num: i32) -> () {
    let mut buf: [u8; 10] = [0; 10];
    let mut i: usize = 0;

    if num < 0 {
        put_char(b'-');
        num = -num;
    }
    if num == 0 {
        put_char(b'0');
        return;
    }
    while num > 0 {
        buf[i] = b'0' + (num % 10) as u8;
        num /= 10;
        i += 1;
    }
    print(&buf[..i]);
}

fn print_number_u(mut num: u32) -> () {
    let mut buf: [u8; 10] = [0; 10];
    let mut i: usize = 0;

    if num == 0 {
        put_char(b'0');
        return;
    }
    while num > 0 {
        buf[i] = b'0' + (num % 10) as u8;
        num /= 10;
        i += 1;
    }
    print(&buf[..i]);
}

