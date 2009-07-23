dnl This file generates Glulx assembly code for dispatching Glk function calls
dnl through the Glulx glk opcode. Each function stub copies its arguments from
dnl the C call stack to the Glx stack and then executes a glk instruction with
dnl the approriate selector.
dnl
define(`fst', `substr(`$1', 0, 1)')dnl
define(`lst', `substr(`$1', eval(len(`$1') - 1))')dnl
define(`narg', `eval(fst(`$1') - ifelse(lst(`$1'), `:', 0, 1))')dnl
define(`stub', `dnl
section code
export glk_`$2'
:glk_`$2'
	dc.b 0xc1 4 1 0 0
copyargs(narg(`$3'))dnl
	glk $1 narg(`$3') (sp)
	return (sp)
')dnl
define(`copyargs', `ifelse(`$1', `0', `', `dnl
	aload {0} -`$1' (sp)
copyargs(eval(`$1'-1))')')dnl
changecom(`;')dnl
dnl
; Glk function stubs generated from glk.m4 -- do not edit this file directly!

;
; Main GLK functions
;

stub(`0x0001', `exit', `0:')
stub(`0x0003', `tick', `0:')
stub(`0x0004', `gestalt', `3IuIu:Iu')
stub(`0x0005', `gestalt_ext', `4IuIu&#Iu:Iu')
stub(`0x0020', `window_iterate', `3Qa<Iu:Qa')
stub(`0x0021', `window_get_rock', `2Qa:Iu')
stub(`0x0022', `window_get_root', `1:Qa')
stub(`0x0023', `window_open', `6QaIuIuIuIu:Qa')
stub(`0x0024', `window_close', `2Qa<[2IuIu]:')
stub(`0x0025', `window_get_size', `3Qa<Iu<Iu:')
stub(`0x0026', `window_set_arrangement', `4QaIuIuQa:')
stub(`0x0027', `window_get_arrangement', `4Qa<Iu<Iu<Qa:')
stub(`0x0028', `window_get_type', `2Qa:Iu')
stub(`0x0029', `window_get_parent', `2Qa:Qa')
stub(`0x002A', `window_clear', `1Qa:')
stub(`0x002B', `window_move_cursor', `3QaIuIu:')
stub(`0x002C', `window_get_stream', `2Qa:Qb')
stub(`0x002D', `window_set_echo_stream', `2QaQb:')
stub(`0x002E', `window_get_echo_stream', `2Qa:Qb')
stub(`0x002F', `set_window', `1Qa:')
stub(`0x0030', `window_get_sibling', `2Qa:Qa')
stub(`0x0040', `stream_iterate', `3Qb<Iu:Qb')
stub(`0x0041', `stream_get_rock', `2Qb:Iu')
stub(`0x0042', `stream_open_file', `4QcIuIu:Qb')
stub(`0x0043', `stream_open_memory', `4&+#!CnIuIu:Qb')
stub(`0x0044', `stream_close', `2Qb<[2IuIu]:')
stub(`0x0045', `stream_set_position', `3QbIsIu:')
stub(`0x0046', `stream_get_position', `2Qb:Iu')
stub(`0x0047', `stream_set_current', `1Qb:')
stub(`0x0048', `stream_get_current', `1:Qb')
stub(`0x0060', `fileref_create_temp', `3IuIu:Qc')
stub(`0x0061', `fileref_create_by_name', `4IuSIu:Qc')
stub(`0x0062', `fileref_create_by_prompt', `4IuIuIu:Qc')
stub(`0x0063', `fileref_destroy', `1Qc:')
stub(`0x0064', `fileref_iterate', `3Qc<Iu:Qc')
stub(`0x0065', `fileref_get_rock', `2Qc:Iu')
stub(`0x0066', `fileref_delete_file', `1Qc:')
stub(`0x0067', `fileref_does_file_exist', `2Qc:Iu')
stub(`0x0068', `fileref_create_from_fileref', `4IuQcIu:Qc')
stub(`0x0080', `put_char', `1Cu:')
stub(`0x0081', `put_char_stream', `2QbCu:')
stub(`0x0082', `put_string', `1S:')
stub(`0x0083', `put_string_stream', `2QbS:')
stub(`0x0084', `put_buffer', `1>+#Cn:')
stub(`0x0085', `put_buffer_stream', `2Qb>+#Cn:')
stub(`0x0086', `set_style', `1Iu:')
stub(`0x0087', `set_style_stream', `2QbIu:')
stub(`0x0090', `get_char_stream', `2Qb:Is')
stub(`0x0091', `get_line_stream', `3Qb<+#Cn:Iu')
stub(`0x0092', `get_buffer_stream', `3Qb<+#Cn:Iu')
stub(`0x00A0', `char_to_lower', `2Cu:Cu')
stub(`0x00A1', `char_to_upper', `2Cu:Cu')
stub(`0x00B0', `stylehint_set', `4IuIuIuIs:')
stub(`0x00B1', `stylehint_clear', `3IuIuIu:')
stub(`0x00B2', `style_distinguish', `4QaIuIu:Iu')
stub(`0x00B3', `style_measure', `5QaIuIu<Iu:Iu')
stub(`0x00C0', `select', `1<+[4IuQaIuIu]:')
stub(`0x00C1', `select_poll', `1<+[4IuQaIuIu]:')
stub(`0x00D0', `request_line_event', `3Qa&+#!CnIu:')
stub(`0x00D1', `cancel_line_event', `2Qa<[4IuQaIuIu]:')
stub(`0x00D2', `request_char_event', `1Qa:')
stub(`0x00D3', `cancel_char_event', `1Qa:')
stub(`0x00D4', `request_mouse_event', `1Qa:')
stub(`0x00D5', `cancel_mouse_event', `1Qa:')
stub(`0x00D6', `request_timer_events', `1Iu:')

;
; GLK Image module
;

stub(`0x00E0', `image_get_info', `4Iu<Iu<Iu:Iu')
stub(`0x00E1', `image_draw', `5QaIuIsIs:Iu')
stub(`0x00E2', `image_draw_scaled', `7QaIuIsIsIuIu:Iu')
stub(`0x00E8', `window_flow_break', `1Qa:')
stub(`0x00E9', `window_erase_rect', `5QaIsIsIuIu:')
stub(`0x00EA', `window_fill_rect', `6QaIuIsIsIuIu:')
stub(`0x00EB', `window_set_background_color', `2QaIu:')

;
; GLK Sound module
;

stub(`0x00F0', `schannel_iterate', `3Qd<Iu:Qd')
stub(`0x00F1', `schannel_get_rock', `2Qd:Iu')
stub(`0x00F2', `schannel_create', `2Iu:Qd')
stub(`0x00F3', `schannel_destroy', `1Qd:')
stub(`0x00F8', `schannel_play', `3QdIu:Iu')
stub(`0x00F9', `schannel_play_ext', `5QdIuIuIu:Iu')
stub(`0x00FA', `schannel_stop', `1Qd:')
stub(`0x00FB', `schannel_set_volume', `2QdIu:')
stub(`0x00FC', `sound_load_hint', `2IuIu:')

;
; GLK Hyperlinks module
;

stub(`0x0100', `set_hyperlink', `1Iu:')
stub(`0x0101', `set_hyperlink_stream', `2QbIu:')
stub(`0x0102', `request_hyperlink_event', `1Qa:')
stub(`0x0103', `cancel_hyperlink_event', `1Qa:')

;
; GLK Unicode module
;

stub(`0x0120', `buffer_to_lower_case_uni', `3&+#IuIu:Iu')
stub(`0x0121', `buffer_to_upper_case_uni', `3&+#IuIu:Iu')
stub(`0x0122', `buffer_to_title_case_uni', `4&+#IuIuIu:Iu')
stub(`0x0128', `put_char_uni', `1Iu:')
stub(`0x0129', `put_string_uni', `1U:')
stub(`0x012A', `put_buffer_uni', `1>+#Iu:')
stub(`0x012B', `put_char_stream_uni', `2QbIu:')
stub(`0x012C', `put_string_stream_uni', `2QbU:')
stub(`0x012D', `put_buffer_stream_uni', `2Qb>+#Iu:')
stub(`0x0130', `get_char_stream_uni', `2Qb:Is')
stub(`0x0131', `get_buffer_stream_uni', `3Qb<+#Iu:Iu')
stub(`0x0132', `get_line_stream_uni', `3Qb<+#Iu:Iu')
stub(`0x0138', `stream_open_file_uni', `4QcIuIu:Qb')
stub(`0x0139', `stream_open_memory_uni', `4&+#!IuIuIu:Qb')
stub(`0x0140', `request_char_event_uni', `1Qa:')
stub(`0x0141', `request_line_event_uni', `3Qa&+#!IuIu:')

; EOF
