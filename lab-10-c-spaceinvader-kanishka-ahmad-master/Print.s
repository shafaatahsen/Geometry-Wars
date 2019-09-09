; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
;; --UUU-- Complete this (copy from Lab7-8)

INPUT 		EQU 0
OFFset 		EQU 4
NUM			EQU 8
ASCII 		EQU 48
		CMP R0, #0
		BEQ zero 
		
		SUB SP, #24 ; Allocate
		MOV R12, SP
		STR R0, [R12,#INPUT]
		MOV R0, #23
		STR R0, [R12,#OFFset]
		MOV R0,#ASCII
		STRB R0, [R12,#23] ; initialize first value to 0
		MOV R0, #0
		STRB R0, [R12,#24] ;initialize null character
		
		LDR R0, [R12,#INPUT]
				
		
Check10	CMP R0, #0		;check if less than 10
		BEQ LCDPOP
		MOV R3, #10
		UDIV R1, R0, R3  ;divide input by 10  e.g. 679(R0)/10(r3) = 67(r1)
		MUL R2, R1, R3   ;get largest factor of 10 that divide input (10*largest number that div input) R2 =670
		SUB R0, R2       ;modulo, remainder of 10 (679-670=9)
		
		LDR R3, [R12, #OFFset] 
		ADD R0, #ASCII
		STRB R0, [R12, R3]      ;store on stack
		
		SUB R3, #1				;go up stack 
		STR R3, [R12, #OFFset]
		
		MOV R0, R1
		B Check10
		
LCDPOP	LDR R0, [R12, #OFFset]
		ADD R0, R12
		ADD R0, #1				; location is one up because of decrement before call
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutString
		POP {R0, R1, R2, LR}
		
		
		
DONE	ADD SP, #24 ; Deallocate
		BX  LR
		
zero	MOV R0, #ASCII          ; zero exception
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		

    BX LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
;; --UUU-- Complete this (copy from Lab7-8)
INPUT2 		EQU 0
OFFset2 	EQU 4
NUM2		EQU 8
		
		SUB SP, #16 ; Allocate
		MOV R12, SP
		STR R0, [R12,#INPUT2]
		MOV R0, #16
		STR R0, [R12,#OFFset2]
		
		LDR R0, [R12, #INPUT2]
		MOV R1, #9999          ; check if greater than 9.999, then output to *.***
		CMP R0, R1
		BHI TooMUCH 
		
		MOV R0,#ASCII
		STRB R0, [R12,#16] ;initialize values to 0
		STRB R0, [R12,#15] 
		STRB R0, [R12,#14]
		STRB R0, [R12,#13] 
		
		LDR R0, [R12, #INPUT2]
		CMP R0, #0
		BEQ LCDPOP2 
				
		
Check01	CMP R0, #0		;check if less than 10
		BEQ LCDPOP2
		MOV R3, #10
		UDIV R1, R0, R3  ;divide input by 10  e.g. 679(R0)/10(r3) = 67(r1)
		MUL R2, R1, R3   ;get largest factor of 10 that divide input (10*largest number that div input) R2 =670
		SUB R0, R2       ;modulo, remainder of 10 (679-670=9)
		
		LDR R3, [R12, #OFFset2] 
		ADD R0, #ASCII
		STRB R0, [R12, R3]      ;store on stack
		
		SUB R3, #1				;go up stack 
		STR R3, [R12, #OFFset2]
		
		MOV R0, R1
		B Check01
		
LCDPOP2	LDRB R0, [R12, #13]  ; OUT the fixed point value most significant bit
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		
		MOV R0, #46
		PUSH {R0, R1, R2, LR} ; out the period
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		
		LDRB R0, [R12, #14]
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		
		LDRB R0, [R12, #15]
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		
		LDRB R0, [R12, #16]
		PUSH {R0, R1, R2, LR}
		BL ST7735_OutChar
		POP {R0, R1, R2, LR}
		
		
		
DONE2	ADD SP, #16 ; Deallocate
		BX  LR
						
TooMUCH MOV R0, #42
		STRB R0, [R12, #13]
		STRB R0, [R12, #14]
		STRB R0, [R12, #15]
		STRB R0, [R12, #16]

		B LCDPOP2
;* * * * * * * * End of LCD_OutFix * * * * * * * *

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
