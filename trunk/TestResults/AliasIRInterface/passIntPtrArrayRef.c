Found a NULL_FILE name in node->get_file_info() for node = SgReturnStmt 
Found a NULL_FILE name in node->get_file_info() for node = SgReturnStmt 
Found a NULL_FILE name in node->get_file_info() for node = SgReturnStmt 
Found a NULL_FILE name in node->get_file_info() for node = SgReturnStmt 
Found a NULL_FILE name in node->get_file_info() for node = SgReturnStmt 

PROCEDURE = { < ProcHandle("blah"), SymHandle("blah") > }
    FORMALS = {
        [
            < 0, SymHandle("s") >
            < 1, SymHandle("t") >
        ] }
    MEMREFEXPRS = { StmtHandle("int blah(int *s,int *t){if((s[0]) ==(t[0])) {}else {}return 0;}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{if((s[0]) ==(t[0])) {}else {}return 0;}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("if((s[0]) ==(t[0])) {}else {}") =>
        [
            MemRefHandle("s") => 
                NamedRef( USE, SymHandle("s"), F, full)
            MemRefHandle("t") => 
                NamedRef( USE, SymHandle("t"), F, full)
            MemRefHandle("(s[0])") => 
                Deref( USE, NamedRef( USE, SymHandle("s"), F, full), 1, F, part)
            MemRefHandle("(t[0])") => 
                Deref( USE, NamedRef( USE, SymHandle("t"), F, full), 1, F, part)
        ] }
    MEMREFEXPRS = { StmtHandle("{}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("return 0;") =>
        [
        ] }
    LOCATION = { < SymHandle("s"), local > }
    LOCATION = { < SymHandle("t"), local > }
    LOCATION = { < SymHandle("blah"), not local > }

PROCEDURE = { < ProcHandle("main"), SymHandle("main") > }
    MEMREFEXPRS = { StmtHandle("int main(){int a;int x = blah(&a,&a);}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{int a;int x = blah(&a,&a);}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("int a;") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("int x = blah(&a,&a);") =>
        [
            MemRefHandle("x") => 
                NamedRef( DEF, SymHandle("x"), F, full)
            MemRefHandle("&a") => 
                NamedRef( USE, SymHandle("a"), T, full)
            MemRefHandle("&a") => 
                NamedRef( USE, SymHandle("a"), T, full)
        ] }
    CALLSITES = { StmtHandle("int x = blah(&a,&a);") =>
        [
            CallHandle("blah(&a,&a)") => 
                NamedRef( USE, SymHandle("blah"), F, full)
        ] }
    PARAMBINDPTRASSIGNPAIRS = { CallHandle("blah(&a,&a)") =>
        [
            < 0, NamedRef( USE, SymHandle("a"), T, full) >
            < 1, NamedRef( USE, SymHandle("a"), T, full) >
        ] }
    MEMREFEXPRS = { StmtHandle("return 0;") =>
        [
        ] }
    LOCATION = { < SymHandle("a"), local > }
    LOCATION = { < SymHandle("x"), local > }
    LOCATION = { < SymHandle("blah"), not local > }
    LOCATION = { < SymHandle("main"), not local > }
