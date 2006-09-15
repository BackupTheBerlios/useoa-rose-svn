
PROCEDURE = { < ProcHandle("foo::foo"), SymHandle("method:foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_") > }
    FORMALS = {
        [
            < 0, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_") >
            < 1, SymHandle("x") >
        ] }
    MEMREFEXPRS = { StmtHandle("") =>
        [
            MemRefHandle("someRef&") => 
                FieldAccess( DEF, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someRef, F, full)
            MemRefHandle("someOtherRef&") => 
                FieldAccess( DEF, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someOtherRef, F, full)
            MemRefHandle("constRef&") => 
                FieldAccess( DEF, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), constRef, F, full)
            MemRefHandle("x") => 
                NamedRef( USE, SymHandle("x"), T, full)
            MemRefHandle("assign_or_agg_initializer:x") => 
                NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full)
            MemRefHandle("assign_or_agg_initializer:((this) -> someRef)") => 
                NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full)
            MemRefHandle("assign_or_agg_initializer:5") => 
                NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full)
            MemRefHandle("(this)") => 
                NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full)
            MemRefHandle("((this) -> someRef)") => 
                FieldAccess( USE, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someRef, F, full)
        ] }
    PTRASSIGNPAIRS = { StmtHandle("") =>
        [
            < FieldAccess( DEF, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someOtherRef, F, full)
            , FieldAccess( USE, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someRef, F, full) >
            < FieldAccess( DEF, Deref( USE, NamedRef( USE, SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), F, full), 1, F, full), someRef, F, full)
            , NamedRef( USE, SymHandle("x"), T, full) >
        ] }
    MEMREFEXPRS = { StmtHandle("public: inline foo(int x) : someRef(x), someOtherRef(((this) -> someRef)), constRef(5){}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{}") =>
        [
        ] }
    LOCATION = { < SymHandle("x"), local > }
    LOCATION = { < SymHandle("method:foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), not local > }
    LOCATION = { < SymHandle("this::foo::foo__foo__scope__foo____MFb_foo__Fb_v_Gb_i_Fe_"), local > }

PROCEDURE = { < ProcHandle("foo::~foo"), SymHandle("method:foo::~foo__foo__scope____dtfoo____MFb_foo__Fb_v_Gb__Fe_") > }
    FORMALS = {
        [
            < 0, SymHandle("this::foo::~foo__foo__scope____dtfoo____MFb_foo__Fb_v_Gb__Fe_") >
        ] }
    MEMREFEXPRS = { StmtHandle("") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("public: inline ~foo(){}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{}") =>
        [
        ] }
    LOCATION = { < SymHandle("method:foo::~foo__foo__scope____dtfoo____MFb_foo__Fb_v_Gb__Fe_"), not local > }
    LOCATION = { < SymHandle("this::foo::~foo__foo__scope____dtfoo____MFb_foo__Fb_v_Gb__Fe_"), local > }

PROCEDURE = { < ProcHandle("foo::operator="), SymHandle("method:foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_") > }
    FORMALS = {
        [
            < 0, SymHandle("this::foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_") >
            < 1, SymHandle("f") >
        ] }
    MEMREFEXPRS = { StmtHandle("") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("public: inline foo &operator=(class foo &f){return *(this);}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{return *(this);}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("return *(this);") =>
        [
            MemRefHandle("(this)") => 
                NamedRef( USE, SymHandle("this::foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), F, full)
            MemRefHandle("*(this)") => 
                NamedRef( USE, SymHandle("this::foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), F, full)
        ] }
    PTRASSIGNPAIRS = { StmtHandle("return *(this);") =>
        [
            < NamedRef( DEF, SymHandle("method:foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), F, full)
            , NamedRef( USE, SymHandle("this::foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), F, full) >
        ] }
    LOCATION = { < SymHandle("f"), local > }
    LOCATION = { < SymHandle("method:foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), not local > }
    LOCATION = { < SymHandle("this::foo::operator=__foo__scope__operator__as____MFb_foo__Fb___Rb__foo__Re___Gb___Rb__foo__Re___Fe_"), local > }

PROCEDURE = { < ProcHandle("main"), SymHandle("main") > }
    MEMREFEXPRS = { StmtHandle("int main(){return 0;}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("{return 0;}") =>
        [
        ] }
    MEMREFEXPRS = { StmtHandle("return 0;") =>
        [
        ] }
    LOCATION = { < SymHandle("main"), not local > }
