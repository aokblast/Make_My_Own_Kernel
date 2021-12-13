  org 0x7c00
stackBase equ 0x7c00
loaderBase equ 0x1000
loaderOffset equ 0x00

rootDirSecs  equ 14
SecNumOfRootDirStart equ 19
SecNumOfFAT1Start equ 1
SecBal equ 17

jmp short Label_Start
nop
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

; set base pointer
Label_Start:
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, stackBase

; cls
  mov ax, 0600h
  mov bx, 0700h
  mov cx, 0000h
  mov dx, 184fh
  int 10h

; set focus
  mov ax, 0200h
  mov bx, 0000h
  mov dx, 0000h
  int 10h

; display msg

  mov ax, 1301h
  mov bx, 000fh
  mov dx, 0000h
  mov cx, 13
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, startBootMsg
  int 10h

; reset floppy

  xor ah, ah
  xor dl, dl
  int 13h

; loader searcher
  mov word [SecNo], SecNumOfRootDirStart
Label_Search_Begin:
  
  cmp word [RootDirSizeForLoop], 0
  jz Label_No_Loader_Bin
  dec word [RootDirSizeForLoop]
  ;set arg for read sector
  xor ax, ax
  mov es, ax
  mov bx, 8000h
  mov ax, [SecNo]
  mov cl, 1
  call Func_ReadSectors

  mov si, LoaderFileName
  mov di, 8000h
  cld

  ; set dir num
  mov dx, 10h

Label_Search_LoaderBin:
  
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
  mov si, LoaderFileName
  ; search the next dir
  jmp Label_Search_LoaderBin

Label_Goto_Next_Sec_In_Root_Dir:
  add word [SecNo], 1
  jmp Label_Search_Begin


; loader not found exception
Label_No_Loader_Bin:
  mov ax, 1301h
  mov bx, 008ch
  mov dx, 0100h
  mov cx, 21
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, NoLoaderMsg
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
 mov ax, loaderBase
 mov es, ax
 mov bx, loaderOffset
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
  call Func_GetFATEntry
  cmp ax, 0fffh
  jz Label_File_Loaded
  push ax
  mov dx, rootDirSecs
  add ax, dx
  add ax, SecBal
  add bx, [BPB_BytesPerSec]
  jmp Label_Loading_File

Label_File_Loaded:

  jmp loaderBase:loaderOffset 


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




; tmp variable
RootDirSizeForLoop dw rootDirSecs
SecNo              dw 0
Odd                db 0

; msgs
startBootMsg:   db "Starting Boot"
NoLoaderMsg:    db "ERROR:NO LOADER Found"
LoaderFileName: db "LOADER  BIN", 0

;fill up to 512
times 510-($ - $$) db 0
dw 0xaa55


