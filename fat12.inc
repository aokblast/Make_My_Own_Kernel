rootDirSecs  equ 14
SecNumOfRootDirStart equ 19
SecNumOfFAT1Start equ 1
SecBal equ 17

BS_OEMName      db 'LinuBoot'
BPB_BytesPerSec dw 512
BPB_SecPerClus  db 1
BPB_ResvdSecCnt dw 1
BPB_NumFATs     db 2
BPB_RootEntCnt  dw 224
BPB_TotSec16    dw 2880
BPB_Media       db 0xf0
BPB_FATSz16     dw 9
BPB_SecPerTrk   dw 18
BPB_NumHeads    dw 2
BPB_hiddSec     dd 0
BPB_TotSec32    dd 0
BS_DrvNum       db 0
BS_Reserved1    db 0
BS_BootSig      db 29h
BS_VolID        dd 0
VS_VolLab       db 'boot loader'
BS_FileSysType  db 'FAT12   '