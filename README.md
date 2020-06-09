# Execution

To compile and run the program, use the following command:

```shell
flex lang.l && yacc -d lang.y && gcc y.tab.c lex.yy.c compiler.c && a < test1.txt
```


symbol-table = array of symbol;
all-symbol-tables = stack of symbol-table;

search_symbol("a", current_symbol_table)
