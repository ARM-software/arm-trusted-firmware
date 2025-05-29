/* Allocate space for DTB binary
   in .rodata section. Post build
   script fills the array.
*/
.section .rodata
.global dtb_array
.global dtb_array_end
dtb_array:
.ALIGN(8);
.skip DTB_ARRAY_SIZE
dtb_array_end = .;
