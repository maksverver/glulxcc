section data

export glulx_header
:glulx_header
	dc.l 0

section code
define(`return_var', `ifelse(`$1', `void', `', ` bp')')dnl
define(`return_arg', `ifelse(`$1', `void', `0', `bp')')dnl
define(`var_name', `regexp(`$1', ` \*?\(\w+\)', `\1')')dnl
define(`var_names',
	`ifelse(`$1',`',`', ` var_name(`$1')`'var_names(shift($@))')')dnl
define(`load_vars',
	`ifelse(`$2',`',`', `
	aload bp -`$1' var_name(`$2')`'dnl
	load_vars(eval(`$1'+1), shift(shift($@)))')')dnl
define(`func', `
export glulx_$2
func_local glulx_$2 bp`'var_names($3)`'load_vars(1, $3)
	$2`'var_names($3)`'return_var($1)
	return return_arg($1)')dnl
include(`glulx-operations.m4')dnl

; EOF
