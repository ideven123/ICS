; 不完全的错误类型
; 指令用错
ADDI R1,R2,R3 
JR   LABLE
;参数个数出错
NOT R1
LDR R1,R2
LDI R1,R2,#0,R5
;参数类型出错.
ST  R4,R5
STR R1,R2,R3
JSRR LABLE1
TRAP R1
BR  R3
;未知检测，可能与LC3的行为 不同 ,对立即数的提取还有问题
ADD R1,R2,#45r
ADD R1,R2,45
ADD R3,R4,xfag
.stringz hllow world
