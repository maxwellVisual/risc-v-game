OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_entry)
SEARCH_DIR("/usr/local/riscv32-unknown-elf/lib");
MEMORY
{
  RAM(rwx): ORIGIN = 0x00000, LENGTH = 0x1ffff
}
SECTIONS
{
  . = 0x0;
  .init : {*(.init)}
  .text : {*(.text)}
  . = ALIGN(4);
  PROVIDE( __global_pointer$ = .);
  .data ALIGN(4) : {*(.data)}
  .bss ALIGN(4) : {
    PROVIDE( __bss_start = .);
    *(.bss)
  }
  . = ALIGN(4);
  PROVIDE( _end = .);
  /* .stack : {
    . = ; */
    PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));
  /* } */
}