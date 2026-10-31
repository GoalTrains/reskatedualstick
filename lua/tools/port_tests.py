from port_translate import *
import subprocess,sys,os
src=Path(sys.argv[1]); output=Path(sys.argv[2]); output.mkdir(exist_ok=True)
selected=['grind_ground_contact','grind_input_fidelity','grind_state701','rear_rail_rotation','pop_air_handoff','pop_handoff_setups','grind_flat_side','manual_touchdown_frame','manual_native_frame','manual_latch_neutral','manual_air_frame','split_grind_exits','release_flick','stick_steering_angle','switch_directions','r3_pump','revert_prewind_options','handplant_hold','stock_early_ramp','goofy_rail_scoop_frame','grind_entry_hold','shuv_steering','scoop_slide_priority','manual_catch','manual_catch_continuations','manual_catch_flick_completion','manual_catch_hold_windows','manual_catch_late','manual_catch_rail_exits','manual_catch_recognition_gap','manual_catch_scoops','camera_dpad','stance_regression','truefoot_late_revert','xl_physical_stance','xl_fakie_air','xl_continuity','xl_frame_handoff','motion_stance','fakie_input_guard','stance_neutral_timing','grind_entry_plan','xl_stance_agreement','xl_basis_agreement']
base=(P.parent/'plugin'/'translate.lua').read_text().replace('return M\n','')
for test in selected:
 text=(src/'test'/(test+'.c')).read_text()
 text=re.sub(r'/\*.*?\*/|//[^\n]*','',text,flags=re.S)
 text=re.sub(r'^#include[^\n]*','',text,flags=re.M)
 text=subprocess.run([os.environ.get('CLANG','clang'),'-E','-P','-imacros',str(P/'source.c'),'-x','c','-'],input=text,text=True,capture_output=True,check=True).stdout
 # Tests are our own C source, with known translator typedefs supplied for parsing.
 tree=c_parser.CParser().parse(''.join('typedef int '+name+';' for name in structs)+'typedef int FILE;\n'+text)
 emit=Emit()
 for n in tree.ext:
  if isinstance(n,A.FuncDef):funcs[n.decl.name]=n
  elif isinstance(n,A.Decl) and n.name:globals[n.name]=typename(n.type)
 for n in tree.ext:
  if isinstance(n,A.FuncDef):emit.function(n.decl.name,n)
  elif isinstance(n,A.Decl):emit.stmt(n)
 (output/(test+'.luau')).write_text(base+'\nlocal function noop() end\nlocal function memcmp(a,b) for k,v in a do if b[k] ~= v then return 1 end end; return 0 end\n'+'\n'.join(emit.lines)+'\nmain()\nreturn true\n',encoding='utf8')
 print(test)
