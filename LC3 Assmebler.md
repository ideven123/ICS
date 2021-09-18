###  LC3 Assmebler

#### Pass one: 识别操作码

> `BR`  匹配一个参数 lable

> `ADD`  此时要匹配三个参数，两个为Ri，最后一个是立即数或者Ri                

> `LD`   匹配两个参数，Ri + lable 

> `ST`  匹配两个参数，Ri + lable 

> `JSR`  匹配一个参数，lable

> `JSRR` 匹配一个参数 ，Ri

> `AND`  此时要匹配三个参数，两个为Ri，最后一个是立即数或者Ri  

> `LDR`   匹配三个参数，Ri +Ri+  imm 

>  `STR`   匹配三个参数，Ri +Ri+  imm 

> `RTI`  无参数

> `NOT`  匹配两个参数，Ri + Ri

> `LDI` 匹配两个参数，Ri + lable

> `STI` 匹配两个参数，Ri + lable 

> `JMP` 匹配一个参数 ，Ri

> `LEA` 匹配两个参数，Ri + lable 

> `TRAP` 匹配一个参数 ,imm 

> `.ORIG` 匹配一个参数 ，imm

> `.END`  无参数

> `.BLKW` 匹配 立即数 。 Imm  可无 # 

> `.FILL `             匹配立即数 imm

> `.STRINGZ`      匹配一个参数  引号串

> ` GETC` 无参数

> `OUT` 无参数

> `PUTS` 无参数

> `IN` 无参数

> `HALT` 无参数

> `GETS` 无参数



分类：

 无参数：`GECT`  `OUT` `PUTS` `IN` `HALT` `GETS` `.END` `RTI` 

 一个参数 Ri：`JSRR`  `JMP`

一个参数 imm : `TRAP` `.ORIG `  `.BLKW`  `.FILL`  

 一个参数 lable : `JSR`  `BR` 

一个参数 strings ： `.stringz`

两个参数  Ri + lable : `LD` `ST` `LDI ` `STI`  `LEA` 

两个参数 Ri + Ri :  `NOT` 

三个参数 Ri + Ri + Ri ：`ADD`  `AND`



==**问题**==

	+  .stringz对有引号，和无引号的识别一样。 
	+ JSR 与JSRR 不同







