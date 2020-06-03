# Execution

To compile and run the program, use the following command:

```shell
flex lang.l && yacc -d lang.y && gcc y.tab.c lex.yy.c && a < test1.txt
```