org 10000h


  jmp Label_Start

  %include "fat12.inc"

  KernelFileBase equ 0x00
  KernelFileOffset equ 0x100000

  KernelAddrBaseTmp equ 0x00
  KernelFileOffsetTmp equ 0x7E00

  memStructBuffAddr equ 0x7E00



[SECTION gtd]

LABEL_GDT:         dd 0, 0
LABEL_DESC_CODE32: dd 0x0000FFFF, 0x00CF9A00
LABEL_DESC_DATA32: dd 0x0000FFFF, 0x00CF9200

GdtLen equ $ - LABEL_GDT
GdtPtr dw GdtLen - 1
       dd LABEL_GDT

selCode32 equ LABEL_DESC_CODE32 - LABEL_GDT
selData32 equ LABEL_DESC_DATA32 - LABEL_GDT

[SECTION gdt64]

LABEL_GDT64:        dq 0x0000000000000000
LABEL_DESC_CODE64:  dq 0x0020980000000000
LABEL_DESC_DATA64:  dq 0x0000920000000000

GdtLen64            equ $ - LABEL_GDT64
GdtPtr64            dw  GdtLen64 - 1
                    dd  LABEL_GDT64

selCode64           equ LABEL_DESC_CODE64 - LABEL_GDT64
selData64           equ LABEL_DESC_DATA64 - LABEL_GDT64


[SECTION .s16]
[BITS 16]

Label_Start:
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ax, 0x00
  mov ss, ax
  mov sp, 0x7c00

; display loader start

  mov ax, 1301h
  mov bx, 000fh
  mov dx, 0200h ; row 2
  mov cx, 12
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, startLoaderMsg
  int 10h

; open A20

  push ax
  in al, 92h
  or al, 00000010b
  out 92h, al
  pop ax

  cli

  lgdt [GdtPtr]
  
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  mov ax, selData32
  mov fs, ax
  mov eax, cr0
  and al, 11111110b
  mov cr0, eax

  sti

; reset floppy

  xor ah, ah
  xor dl, dl
  int 13h

; Kernel searcher
  mov word [SecNo], SecNumOfRootDirStart
Label_Search_Begin:
  
  cmp word [RootDirSizeForLoop], 0
  jz Label_No_Kernel_Bin
  dec word [RootDirSizeForLoop]
  ;set arg for read sector
  xor ax, ax
  mov es, ax
  mov bx, 8000h
  mov ax, [SecNo]
  mov cl, 1
  call Func_ReadSectors

  mov si, KernelFileName
  mov di, 8000h
  cld

  ; set dir num
  mov dx, 10h

Label_Search_KernelBin:
  
  cmp dx, 0
  jz Label_Goto_Next_Sec_In_Root_Dir
  dec dx

  ;set fileName size
  mov cx, 11

Label_Cmp_FileName:
  
  cmp cx, 0
  jz Label_FileFound
  dec cx
  lodsb
  cmp al, byte [es:di]
  jz Label_Go_On
  jmp Label_Diff

Label_Go_On:
  
  inc di
  jmp Label_Cmp_FileName

Label_Diff:
  
  and di, 0ffe0h
  add di, 20h
  mov si, KernelFileName
  ; search the next dir
  jmp Label_Search_KernelBin

Label_Goto_Next_Sec_In_Root_Dir:
  add word [SecNo], 1
  jmp Label_Search_Begin


; Kernel not found exception
Label_No_Kernel_Bin:
  mov ax, 1301h
  mov bx, 008ch
  mov dx, 0300h
  mov cx, 22
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, noKernelMsg
  int 10h
  jmp $


; loader loader

Label_FileFound:
 mov ax, rootDirSecs
 and di, 0ffe0h
 add di, 01ah
 mov cx, word [es:di]
 push cx
 add cx, ax
 add cx, SecBal
 mov ax, KernelAddrBaseTmp
 mov es, ax
 mov bx, KernelFileOffsetTmp
 mov ax, cx

Label_Loading_File:

; print Loading
  push ax
  push bx
  mov ah, 0eh
  mov al, '.'
  mov bl, 0fh
  int 10h
  pop bx
  pop ax

  mov cl, 1
  call Func_ReadSectors
  pop ax

;
  push cx
  push eax
  push fs
  push edi
  push ds
  push esi

  mov cx, 200h
  mov ax, KernelFileBase
  mov fs, ax
  mov edi, dword [KernelFileCountOffset]
  
  mov ax, KernelAddrBaseTmp
  mov ds, ax
  mov esi, KernelFileOffsetTmp

Label_Mov_Kernel:
  mov al, byte [ds:esi]
  mov byte [fs:edi], al

  inc esi
  inc edi
  
  loop Label_Mov_Kernel

  mov eax, 0x1000
  mov ds, eax

  mov dword [KernelFileCountOffset], edi

  pop esi
  pop ds
  pop edi
  pop fs
  pop eax
  pop cx

;

  call Func_GetFATEntry
  cmp ax, 0fffh
  jz Label_File_Loaded
  push ax
  mov dx, rootDirSecs
  add ax, dx
  add ax, SecBal
  
  jmp Label_Loading_File

Label_File_Loaded:

  mov ax, 0B800h
  mov gs, ax
  mov ah, 0Fh
  mov al, 'G'
  mov [gs:((80 * 0 + 39) * 2)], ax

KillMotor:
  
  push dx
  mov dx, 03F2h
  mov al, 0
  out dx, al
  pop dx

; get memory address size type

; readMsgWrite

  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0400h ; row 4
  mov cx, 44
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, startGetMemStructMsg
  int 10h


  mov ebx, 0
  xor ax, ax
  mov es, ax
  mov di, memStructBuffAddr

Label_Get_Mem_Struct:
  
  mov eax, 0x0E820
  mov ecx, 20
  mov edx, 0x534D4150
  int 15h

  jc Label_Get_Mem_Failed
  add di, 20
  inc dword [memStructNumber]

  cmp ebx, 0
  jne Label_Get_Mem_Struct
  jmp Label_Get_Mem_OK

Label_Get_Mem_Failed:

  mov dword [memStructNumber], 0
  
  mov ax, 1301h
  mov bx, 008Ch
  mov dx, 0500h ;row 5
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, getMemStructErrMsg
  int 10h
  jmp $

Label_Get_Mem_OK:
  
  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0600h
  mov cx, 29
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, getMemStructOKMsg
  int 10h

; set the SVGA mode

; print msg
  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0800h ; row 8
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAInfoVBEMsg
  
  int 10h

  xor ax, ax
  mov es, ax
  mov di, 8000h
  mov ax, 4F00h

  int 10h

  cmp ax, 004Fh
  
  jz .KO

; failed
  mov ax, 1301h
  mov bx, 008ch
  mov dx, 0900h ; row 9
  mov cx, 23
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAInfoVBEErrMsg
  int 10h

  jmp $

.KO:
  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0A00h ; row 10
  mov cx, 29
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAInfoVBEOKMsg
  int 10h
; get SVGAModeInfo
  
  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0C00h ; row 12
  mov cx, 24
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAModeInfoMsg
  int 10h

  xor ax, ax
  mov es, ax
  mov si, 800eh

  mov esi, dword[es:si]
  mov edi, 8200h

Label_SVGA_Mode_Info_Get:
  
  mov cx, word [es:esi]

; display SVGA Mode Info

  push ax

  xor ax, ax
  mov al, ch
  call Func_DispAL

  mov ax, 00h
  mov al, cl
  call Func_DispAL

  pop ax

  cmp cx, 0xffff
  jz Label_SVGA_Mode_Info_Finish
  
  mov ax, 4F01h
  int 10h

  cmp ax, 004Fh

  jnz Label_SVGA_Mode_Info_Failed

  inc dword [SVGAModeCounter]
  add esi, 2
  add edi, 100h

  jmp Label_SVGA_Mode_Info_Get

Label_SVGA_Mode_Info_Failed:

  mov ax, 1301h
  mov bx, 008Ch
  mov dx, 0D00h ; row 13
  mov cx, 24
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAModeInfoErrMsg
  int 10h

  jmp $

Label_SVGA_Mode_Info_Finish:

  mov ax, 1301h
  mov bx, 000Fh
  mov dx, 0E00h ; row 14
  mov cx, 30
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, SVGAModeInfoOKMsg
  int 10h

; set SVGA mode

  mov ax, 4F02h
  mov bx, 4180h
  int 10h

  cmp ax, 004Fh
  jz Label_SET_SVGA_SUCESS
  jmp $

  Label_SET_SVGA_SUCESS:
  
  ; init IDT GTD and goto protect mode

  cli 

  db 0x66
  lgdt [GdtPtr]

  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp dword selCode32:GO_TO_TMP_Protect

[SECTION .32]
[BITS 32]

GO_TO_TMP_Protect:

; go to tmp long mode

  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov ss, ax
  mov esp, 7E00h

  call support_long_mode
  test eax, eax
  jz no_support

  mov dword [0x90000], 0x91007
  mov dword [0x90800], 0x91007
  
  mov dword [0x91000], 0x92007
  
  mov dword [0x92000], 0x000083

  mov dword [0x92008], 0x200083

  mov dword [0x92010], 0x400083

  mov dword [0x92018], 0x600083

  mov dword [0x02020], 0x800083

  mov dword [0x92028], 0xa00083

; load GDTR

  db 0x66
  lgdt [GdtPtr64]
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  mov esp, 7E00h

; open PAE

  mov eax, cr4
  bts eax, 5
  mov cr4, eax

; load cr3
  
  mov eax, 0x90000
  mov cr3, eax

  mov ecx, 0C0000080h
  rdmsr

  bts eax, 8
  wrmsr

; open PE and paging
  
  mov eax, cr0
  bts eax, 0
  bts eax, 31
  mov cr0, eax
  jmp selCode64:KernelFileOffset

support_long_mode:
  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  setnb    al
  jb support_long_mode_done
  mov eax, 0x00000001
  cpuid
  bt edx, 29
  setc al

support_long_mode_done:

  movzx eax, al
  ret

no_support:
  jmp $





[SECTION .s16lib]
[BITS 16]
; read a sector
; ax=start sector(LBA)
; cl=sector num to read
; es:bx=buffer location
Func_ReadSectors:
  push bp
  mov bp, sp
  sub esp, 2
  mov byte [bp - 2], cl
  push bx
  ;LBA to CHS
  mov bl, [BPB_SecPerTrk]
  div bl
  inc ah
  mov cl, ah
  mov dh, al
  shr al, 1
  mov ch, al
  and dh, 1

  pop bx
  mov dl, [BS_DrvNum]
Label_Reading_Sector:
  mov ah, 2
  mov al, byte [bp - 2]
  int 13h
  jc Label_Reading_Sector
  add esp, 2
  pop bp
  ret


; get FAT12 Entry
; ah=FATEntry
Func_GetFATEntry:
  push es
  push bx
  push ax
  xor ax, ax
  mov es, ax
  pop ax
  mov byte [Odd], 0
  mov bx, 3
  mul bx
  mov bx, 2
  div bx
  cmp dx, 0
  jz Label_Even

  mov byte [Odd], 1

Label_Even:
  xor dx, dx
  mov bx, [BPB_BytesPerSec]
  div bx
  push dx
  mov bx, 8000h
  add ax, SecNumOfFAT1Start
  mov cl, 2

  call Func_ReadSectors

  pop dx
  add bx, dx
  mov ax, [es:bx]
  cmp byte [Odd], 1
  jnz Label_Even_2
  shr ax, 4

Label_Even_2:
  and ax, 0fffh
  pop bx
  pop es
  ret

; display num in al
; al=the num you want to display
Func_DispAL:
  push ecx
  push edx
  push edi

  mov edi, [displayPos]
  mov ah, 0Fh
  mov dl, al
  shr al, 4
  mov ecx, 2
.begin:
  
  and al, 0Fh
  cmp al, 9
  ja .1
  add al, '0'
  jmp .2

.1:
  sub al, 0Ah
  add al, 'A'
.2:
  mov [gs:edi], ax
  add edi, 2

  mov al, dl
  loop .begin

  mov [displayPos], edi

  pop edi
  pop edx
  pop ecx

  ret

; tmp IDT

IDT:
  times 0x50 dq 0
IDT_END:

IDT_POINTER:
    dw IDT_END - IDT - 1
    dw IDT


; tmp variable

RootDirSizeForLoop    dw rootDirSecs
SecNo                 dw 0
Odd                   db 0
KernelFileCountOffset dd KernelFileOffset

memStructNumber       dd 0

SVGAModeCounter       dd 0

displayPos            dd 0

; msgs

startLoaderMsg:       db "Start Loader"
noKernelMsg:          db "ERROR: NO KERNAL Found"
KernelFileName:       db "KERNEL  BIN", 0
startGetMemStructMsg: db "Start Get Memory Structure."
getMemStructErrMsg:   db "Get Memory Struct ERROR"
getMemStructOKMsg:    db "Get Memory Struct SUCESSFUL!"

SVGAInfoVBEMsg:       db "Start Get SVGA VBE Info"
SVGAInfoVBEErrMsg:    db "Get SVGA VBE Info ERROR"
SVGAInfoVBEOKMsg:     db "Get SVGA VBE Info SUCESSFUL!"

SVGAModeInfoMsg:      db "Start Get SVGA Mode Info"
SVGAModeInfoErrMsg:   db "Get SVGA Mode Info ERROR"
SVGAModeInfoOKMsg:    db "Get SVGA Mode Info SUCESSFUL!"
