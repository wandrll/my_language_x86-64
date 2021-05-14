
default rel

global fixed_scanf
global fixed_printf

BUFFER_SIZE equ 128

section .text

fix_size equ 24

%macro multipush 1-* 
%rep %0 
          push %1 
%rotate 1 
%endrep 
%endmacro


%macro multipop 1-* 
%rep %0 
%rotate -1 
          pop %1 
%endrep 
%endmacro

;----------------------------------------------------------------------------------------            
;Destroylist:  
;all args in stack (Cdecl)
dq "SCANF_STARTS_HERE"
fixed_scanf:
            multipush r11, r12, rbx, r13, r14, r10, r15
            sub rsp, BUFFER_SIZE                ;Trick to get rid of "section .data". we move rsp far left, so we have huge buffer. Then, we move it back
            mov r15, rsp                        

            mov rax, 0x00                       ;
            mov rdi, 0                          ;
            mov rsi, r15                        ;Load buffer
            mov rdx, BUFFER_SIZE                ;
            syscall                             ;


            xor r12, r12                        ;
            mov rsi, r15                        ;
            mov r12b, byte[rsi]                 ;
            cmp r12b, '-'                       ;Check, if number is negative. If it is negative, set r12b in 1
                jne skip_negative_s             ;
                inc rsi                         ;
                mov r12, 1                      ;
skip_negative_s:                                ;



            xor rax, rax                        ;
            xor rbx, rbx                        ;
            mov r9, 10                          ;
                                                ;
                                                ;
            cmp byte[rsi], '.'                  ;
            je end_int                          ;
                                                ;
begin_int_loop:                                 ;
                cmp byte[rsi], 10               ;If number is integer, it jumps on special label
                    je end_int                  ;
                                                ;
                mul r9                          ;
                mov bl, [rsi]                   ;Handle with intger part and move result in r10
                sub bl, '0'                     ;
                add rax, rbx                    ;
                inc rsi                         ;
                                                ;
                                                ;
                cmp byte[rsi], '.'              ;
            jne begin_int_loop                  ;
                                                ;
            inc rsi                             ;
                                                ;
            mov r10, rax                        ;   
            shl r10, fix_size                   ;
                                                ;
            mov rcx, 10                         ;
                                                
            xor rbx, rbx                        ;
            xor rax, rax                        ;
                                                ;
begin_float_loop:                               ;
                cmp byte[rsi], 10               ;
                    je add_power                ;
                                                ;
                mul r9                          ;Handle with fractional part. If amount of digits less than 10, we must multiply by 10^x, where x is 10 - number of digits
                mov bl, [rsi]                   ;
                sub bl, '0'                     ;
                add rax, rbx                    ;
                inc rsi                         ;
            loop begin_float_loop               ;
                                                ;
            jmp end                             ;

add_power:

add_power_loop:
                mul r9
            loop add_power_loop

end:

            shl rax, fix_size                   ;
            mov r9, 10000000000                ;
            div r9                              ;Make fixed-precision number
                                                ;
            add rax, r10                        ;

            cmp r12b, 1                         ;
            jne skip1                           ;
                not rax                         ;If number is negative
                inc rax                         ;
skip1:


            add rsp, BUFFER_SIZE
            multipop r11, r12, rbx, r13, r14, r10, r15
            ret

end_int:
            shl rax, fix_size                   ;Make fixed-precision number

            cmp r12b, 1                         ;
            jne skip1                           ;
                not rax                         ;If number is negative
                inc rax                         ;
skip2:

            add rsp, BUFFER_SIZE
            multipop r11, r12, rbx, r13, r14, r10, r15
            ret

;;----------------------------------------------------------------------------------------
dq "SCANF_ENDS_HERE"




%define PRINT_SIZE r11

dq "PRINTF_STARTS_HERE"
;----------------------------------------------------------------------------------------            
;Destroylist:  
;all args in stack (Cdecl)
fixed_printf:   
            mov rax, rdi

            multipush r11, rbx, r13, r14, r10, r15
            
            call print_decimal

            multipop r11, rbx, r13, r14, r10, r15
            
            ret
;;----------------------------------------------------------------------------------------
print_decimal:
;;Destroy list: rax, rcx, r14, rbx, rdx, r13
;;               
;;rax - register to print
            sub rsp, BUFFER_SIZE            ;In order to avoid section .data we move rsp far left, so we have huge buffer. Then, we move it back
            mov rdi, rsp

            push rbp
            mov rbp, rsp

            xor PRINT_SIZE, PRINT_SIZE
            mov rbx, 1                      ;
            shl rbx, 63                     ;
            and rbx, rax                    ;    
            shr rbx, 63                     ;
                                            ;
            cmp rbx, 1                      ;Handle negative numbers
            jne skip_negative               ;
                not rax                     ;    
                inc rax                     ;
                                            ;
                mov byte[rdi], '-'          ;
                inc rdi                     ;
                inc PRINT_SIZE              ; 
skip_negative:

            mov r10, rax                    ;
                                            ;
            shl r10, 40                     ;
            shr r10, 40                     ;Divide rax on 2 parts. r10 contains fractioanl part, rax contains interger part
                                            ;
            shr rax, fix_size               ;                                           
                                            ;
            xor rcx, rcx                    
            xor r14, r14                    
                                            
            mov rbx, 10

            lea r13, [Symbols]            

Calculate_10_loop:
                xor edx, edx                ;
                div ebx                     ;
                                            ;
                mov r14b, [r13 + rdx]       ;        
                push r14                    ;
                                            ;
                inc rcx                     ;
                                            ;Calculte and drop in buffer integer part of number
                cmp eax, 0                  ;        
            jne Calculate_10_loop           ;
                                            ;
                                            ;
                                            ;
 Add_in_buffer_loop:                        ;
                pop rax                     ;
                stosb                       ;
                inc PRINT_SIZE              ;
            loop Add_in_buffer_loop         ;       
 



            mov rax, r10                    ;
            xor rdx, rdx                    ;
            mov r9, 100000000             ;
            mul r9                          ;
            shr rax, fix_size               ;
                                            ;
            xor rcx, rcx                    ;
            xor r14, r14                    ;
            mov rbx, 10                     ;
            Calculate_10_loop_:             ;
                xor edx, edx                ;       
                div ebx                     ;
                                            ;Calculte and save in buffer fractional part of a number
                mov r14b, [r13 + rdx]       ;       
                push r14                    ;
                inc rcx                     ;
                cmp eax, 0                  ;        
            jne Calculate_10_loop_          ;
                                            ;
            mov byte [rdi], '.'             ;
            inc rdi                         ;
            inc PRINT_SIZE                  ;


            push rcx                        ;
            mov r9, rcx                     ;
            mov rcx, 8                      ;
            sub rcx, r9                     ;
            je End_add_zeors_loop           ;
Add_zeros_loop:                             ;
                mov byte [rdi], '0'         ;Add extra zeros in fractional part   
                inc rdi                     ;   
                inc PRINT_SIZE              ;
                                            ;
            loop Add_zeros_loop             ;
                                            ;
End_add_zeors_loop:                         ;
            pop rcx                         ;



                                            ;
Add_in_buffer_loop_:                        ;
                pop rax                     ;
                stosb                       ;
                inc PRINT_SIZE              ;
            loop Add_in_buffer_loop_        ;        
 

            mov byte [rdi], 10              ;add '\n' in the end of the buffer
            inc PRINT_SIZE                  ;

            pop rbp

            mov rsi, rsp                    ;
            mov rax, 0x01                   ;
            mov rdi, 1                      ;drop buffer in stdout
            mov rdx, PRINT_SIZE             ;

            syscall

            add rsp, BUFFER_SIZE            ;move rsp back on its noraml position



            ret
;;----------------------------------------------------------------------------------------

Symbols		db '0','1','2','3','4','5','6','7','8','9'

dq "PRINTF_ENDS_HERE"

