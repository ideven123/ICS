[toc]

# LC3 Assmebler

## 功能简介

1. 该LC3汇编器基本上实现了LC3汇编代码，到机器码的翻译过程。其中包含了十六跳条基本指令，加上 伪指令，以及一些系统调用的常用调用名称。如`in`  `out`  `getc` `halt` 。 
2. 一些特性说明：
   + 该汇编器对指令 是大小写不敏感的。 
   + 没有lable型 立即数的越界检查，越界可能会导致该条指令代码出错，需要程序员注意。
   + 对伪操作只检查格式正确性，不做翻译，后来又加上了翻译，但是翻译是在`Write_objfile` 时实现，有点脱离体系。
   + `.string`  声明的内容可以没有双引号，此时默认后面的第一个连续串(无空格)为.str内容
   + 对 `lable` 的要求也没很高 ，只要没有逗号和空格即可。(这个可以在 检查语法时 加进去对label的检查) 
   + 不支持`lable` 和 指令中间换行 
   + br 指令的nzp顺序不能乱
   + blkw 必须有参数，且不检查正负
   + 其余方面尽可能接近官方的汇编结果。

## 实现框架

> 下面按照实现的步骤和逻辑，详细介绍实现细节。其中主要说明实现了那些函数。(实现细节见附录)

### main 函数

`main`  函数的的主要作用是读取参数，分析汇编文件名，创建目标文件。然后调用`pass one `  和 `pass two`  。最后free 调分配的空间包括符号表和句子信息。使用的函数分别为：

+ `check_for_asm_file()`  分析文件名
+ `asm_pass_one()`  第一遍扫描
+ `asm_pass_two()`  第二遍扫描
+ `Free_malloc1()` free句子信息
+ `Free_malloc2()`  free符号表

### pass one

第一遍遍历要做的事情是 ，

+  `line_sprt_by_space()`  读取文件的一行，并按逗号和空格将 各个成分提取出来 

+ `check_line_syntax`   检查每一句的语法。

  具体检查的项目有

  +  `scanf_OPorLB()` 提取出操作码 
  + `symbol_tab_add()` 如果是label则建表。
  + `analyse_op()` 然后针对每一个具体的命令检查，其格式和操作数 。

+ 然后对每一个句子的提取出的信息 用链表保存，

其中会贯穿两个全局变量 `numErrors` ` srcLineNum`  分别记录分析到的位置，和错误数。

### pass two

第二遍扫描时句子信息已经获取完毕切全是正确的 ，只需针对每一条指令翻译即可。

+  `get_bin_inst` 
+ 并将翻译的机器码写入目标文件中。

## 测试

测试文件在项目里，文件里测试的比较详细。分别有前三次实验的正确代码汇编。以及特定的错误集。

##  附录

###  Pass one: 识别操作码

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

1是立即数 ,0 是 寄存器

### Pass two翻译成机器码

根据上面分析的指令格式实现了针对几个参数模式的翻译方法 。函数如下

+ `get_reg_lable_bincode()`
+ `get_reg_reg_regorimm_bincode()`
+ `get_lable_bincode`
+ `get_reg_reg_imm_bincode()`

其他一些较简单的没用用函数实现，而是直接实现了。如参数是一个寄存器类型。

### 错误处理



>  #define ERR_OPEN_READ       "could not open '%s' for reading."

> \#define ERR_OPEN_WRITE      "could not open '%s' for writing."

> \#define ERR_LINE_TOO_LONG   "source line too long (max is %d)"

> \#define ERR_NO_ORIG         "no .ORIG directive found"

> \#define ERR_NO_END          "no .END directive found"

> \#define ERR_ORIG_NOT_1ST    "instruction(s) appear before .ORIG"

> \#define ERR_END_NOT_LAST    "instruction(s) appear after .END"

> \#define ERR_MISSING_ORIG    ".ORIG not found"

> \#define ERR_MISSING_END     ".END not found"

> \#define ERR_EXPECTED_COMMA  "expected comma, got '%s'"

> \#define ERR_EXPECTED_REG    "expected register (R0-R7), got '%s'"

> \#define ERR_EXPECT_REG_IMM  "expected register or immediate, got '%s'"

> \#define ERR_EXPECT_lable_IMM  "expected register or immediate, got '%s'"

> \#define ERR_BAD_LABEL       "label '%s' conflicts "

> \#define ERR_MISSING_OP      "expected LC3 op, got '%s'"

> \#define ERR_MISSING_OPERAND "too few operand(s)"

> \#define ERR_EXTRA_OPERAND   "extra operand(s) '%s'"

> \#define ERR_DUPLICATE_LABEL "label '%s' previosly defined"

> \#define ERR_MISSING_LABEL   "label '%s' never defined"

> \#define ERR_BAD_PCOFFSET    "PCoffset to '%s' out of range"

> \#define ERR_BAD_IMM         "immediate '%s' (bad format)"

> \#define ERR_IMM_TOO_BIG     "immediate '%s' out of range"

> \#define ERR_EXPECTED_STR    "expected quoted string, got '%s'"

> \#define ERR_ILLEAGLE_INST   "illeagle instruction"

> \#define ERR_BAD_STR         "unterminated string '%s'"

> \#define ERR_WRONG_FORMAT    "instruction . parameter doesn't match "

> \#define ERR_EXPECTED_quot   "expect quot to be a string "







