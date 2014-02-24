/*
toolbarItems.h
*/

int addCheckButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd );
int addRadioButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd );
int addMenuButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos );
int addButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos );
int addWidget ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos );
int addSeparator ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos );
int insertItem ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos );
