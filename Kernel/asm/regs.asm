section .text

GLOBAL backupregs 
GLOBAL addregs

    backupregs:

    mov [backrbp], rbp
    ;mov [backrsp], [rsp + ]           ; bajo del return 
    mov [backrsp], rsp 
    mov [backrax] ,rax
    mov [backrdi],rdi
    ret 

    addregs:
    ; Recibe un puntero a RegisterDump en RDI
    
    ; Guardar registros generales (64 bits cada uno, offset 6 para time[6])
    mov rax, [backrax]
    mov [rdi + 6], rax               
    
    mov [rdi + 14], rbx              ; rbx
    mov [rdi + 22], rcx              ; rcx
    mov [rdi + 30], rdx              ; rdx
    mov [rdi + 38], rsi              ; rsi
    
    mov rax, [backrdi]
    mov [rdi + 46], rax              
    
    mov rax, [backrbp]
    mov [rdi + 54], rax             
    
    mov rax, [backrsp]
    mov [rdi + 62], rax             
      
    
    ; Registros R8-R15
    mov [rdi + 70], r8          
    mov [rdi + 78], r9          
    mov [rdi + 86], r10        
    mov [rdi + 94], r11         
    mov [rdi + 102], r12        
    mov [rdi + 110], r13        
    mov [rdi + 118], r14        
    mov [rdi + 126], r15        
    
    ; RIP (usar LEA para obtener dirección actual)
    lea rax, [rel $]
    mov [rdi + 134], rax        
    
    ; RFLAGS
    pushfq
    pop rax
    mov [rdi + 142], rax        
    
    ; Segmentos (16 bits cada uno)
    mov ax, cs
    mov [rdi + 150], ax         
    mov ax, ds
    mov [rdi + 152], ax         
    mov ax, es
    mov [rdi + 154], ax         
    mov ax, fs
    mov [rdi + 156], ax         
    mov ax, gs
    mov [rdi + 158], ax         
    mov ax, ss
    mov [rdi + 160], ax         
    
    ret

    

section .bss
backrax: resq 1                 
backrbp: resq 1                 
backrsp: resq 1 
backrdi: resq 1 

