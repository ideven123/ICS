      .orig x3000
      LEA R0,DATA
      AND R1,R1,#0
      ADD R1,R1,#9
LOOP  ADD R2,R0,#0
      ADD R3,R1,#0
LOOP2 JSR SUB1
      ADD R4,R4,#0
      BRZP LABLE
      JSR SUB2
LABLE ADD R2,R2,#1
      ADD R3,R3,#-1
      BRP LOOP2
      ADD R1, R1 #-1
      BRP LOOP2
      halt
DATA  .BLKW 10 
SUB1  LDR R5,R2,#0
      NOT R5,R5
      ADD R5,R5,#1
      LDR R6,R2,#1
      ADD R4,R5,R6
      RET
SUB2  LDR R4,R2,#0
      LDR R5,R2,#1
      STR R4,R2,#1
      STR R5,R2,#0
      RET
      .end             
