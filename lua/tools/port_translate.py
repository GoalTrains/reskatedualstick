"""Reproducible C-control-state to Luau conversion; no native code is shipped in the mod.
Only our pure control implementation is converted. Game gesture paths are supplied by ReSkate.
"""
from pathlib import Path
from pycparser import c_parser, c_ast as A
import re, json
P=Path(__file__).parent
LUA_KEYWORDS=set('and break do else elseif end false for function if in local nil not or repeat return then true until while continue'.split())
def identifier(name): return '_c_'+name if name in LUA_KEYWORDS else name
ast=c_parser.CParser().parse((P/'source.pp.c').read_text())
structs={}; funcs={}; globals={}; enum={}
def typename(t):
 if isinstance(t,A.TypeDecl): return typename(t.type)
 if isinstance(t,A.IdentifierType):return ' '.join(t.names)
 if isinstance(t,A.PtrDecl):return ('ptr',typename(t.type))
 if isinstance(t,A.ArrayDecl):return ('array',typename(t.type),int(t.dim.value) if isinstance(t.dim,A.Constant) else 0)
 if isinstance(t,A.Struct):return t
 if isinstance(t,A.FuncDecl):return ('func',typename(t.type))
 raise Exception(type(t))
for n in ast.ext:
 if isinstance(n,A.Typedef):
  t=typename(n.type)
  if isinstance(t,A.Struct):structs[n.name]={d.name:typename(d.type) for d in t.decls}
 elif isinstance(n,A.FuncDef):funcs[n.decl.name]=n
 elif isinstance(n,A.Decl):
  if n.name:globals[n.name]=typename(n.type)
  if isinstance(n.type,A.Enum):
   for i,e in enumerate(n.type.values.enumerators):enum[e.name]=i
class Emit:
 def __init__(self):self.types=dict(globals);self.lines=[];self.level=0;self.increment=[];self.refs=set()
 def put(self,s):self.lines.append(('    '*self.level+s).rstrip())
 def typeof(self,n):
  if isinstance(n,A.ID):return self.types.get(n.name,'int')
  if isinstance(n,A.Constant):return n.type
  if isinstance(n,A.CompoundLiteral):return typename(n.type.type)
  if isinstance(n,A.StructRef):
   t=self.typeof(n.name)
   if isinstance(t,tuple) and t[0]=='ptr':t=t[1]
   return structs[t][n.field.name]
  if isinstance(n,A.ArrayRef):return self.typeof(n.name)[1]
  if isinstance(n,A.Cast):return typename(n.to_type.type)
  if isinstance(n,A.UnaryOp):
   if n.op=='*':return self.typeof(n.expr)[1]
   if n.op=='&':return ('ptr',self.typeof(n.expr))
   return self.typeof(n.expr)
  if isinstance(n,A.FuncCall):return typename(funcs[n.name.name].decl.type.type) if n.name.name in funcs else 'float'
  if isinstance(n,A.TernaryOp):return self.typeof(n.iftrue)
  if isinstance(n,A.BinaryOp):
   if n.op in ['==','!=','<','>','<=','>=','&&','||','&','|','%']:return 'int'
   a,b=self.typeof(n.left),self.typeof(n.right)
   return 'double' if 'double' in (a,b) else 'float' if 'float' in (a,b) else 'int'
  if isinstance(n,A.Assignment):return self.typeof(n.lvalue)
  return 'int'
 def truth(self,n):return 'truth('+self.expr(n)+')'
 def convert(self,s,t):
  if t in structs:return 'copy('+s+')'
  if t=='float':return 'f32('+s+')'
  if t in ('int','short','unsigned','unsigned short','unsigned char'):return 'trunc('+s+')'
  return s
 def default(self,t):
  if isinstance(t,str) and t in structs:return 'new_'+t+'()'
  if isinstance(t,tuple):
   if t[0]=='array':return '{'+', '.join('['+str(i)+']='+self.default(t[1]) for i in range(t[2]))+'}'
   return 'nil'
  return '0'
 def init(self,t,n):
  if n is None:return self.default(t)
  if isinstance(n,A.InitList):
   if isinstance(t,str) and t in structs:
    keys=list(structs[t]);values={};cursor=0
    for value in n.exprs:
     if isinstance(value,A.NamedInitializer):
      key=value.name[0].name;values[key]=value.expr;cursor=keys.index(key)+1
     else:
      values[keys[cursor]]=value;cursor+=1
    return '{'+', '.join(k+'='+self.init(ft,values.get(k)) for k,ft in structs[t].items())+'}'
   if isinstance(t,tuple) and t[0]=='array':return '{'+', '.join('['+str(i)+']='+self.init(t[1],n.exprs[i] if i<len(n.exprs) else None) for i in range(t[2]))+'}'
   return self.expr(n.exprs[0])
  return self.convert(self.expr(n),t)
 def expr(self,n):
  if n is None:return ''
  if isinstance(n,A.CompoundLiteral):return self.init(typename(n.type.type),n.init)
  if isinstance(n,A.ID):return str(enum[n.name]) if n.name in enum else identifier(n.name)
  if isinstance(n,A.Constant):
   if n.type=='string':return n.value
   if n.type=='char':return str(ord(n.value[1:-1]))
   return re.sub(r'[uUlL]+$' if n.value.lower().startswith('0x') else r'[fFuUlL]+$','',n.value)
  if isinstance(n,A.StructRef):return self.expr(n.name)+'.'+n.field.name
  if isinstance(n,A.ArrayRef):return self.expr(n.name)+'['+self.expr(n.subscript)+']'
  if isinstance(n,A.BinaryOp):
   a,b=self.expr(n.left),self.expr(n.right);op=n.op
   if op in ['&&','||']:return '(if '+self.truth(n.left)+' then '+ ('bool('+self.truth(n.right)+')' if op=='&&' else '1')+' else '+('0' if op=='&&' else 'bool('+self.truth(n.right)+')')+')'
   if op in ['==','!=','<','>','<=','>=']:return 'bool('+a+' '+('~=' if op=='!=' else op)+' '+b+')'
   if op in ['&','|','^','<<','>>']:return 'bit32.'+{'&':'band','|':'bor','^':'bxor','<<':'lshift','>>':'rshift'}[op]+'('+a+', '+b+')'
   out='('+a+' '+op+' '+b+')'
   if op=='/' and self.typeof(n)=='int':out='trunc('+out+')'
   return out
  if isinstance(n,A.TernaryOp):return '(if '+self.truth(n.cond)+' then '+self.expr(n.iftrue)+' else '+self.expr(n.iffalse)+')'
  if isinstance(n,A.Cast):return self.convert(self.expr(n.expr),typename(n.to_type.type))
  if isinstance(n,A.UnaryOp):
   v=self.expr(n.expr)
   if n.op=='!':return 'bool(not truth('+v+'))'
   if n.op=='-':return '(-'+v+')'
   if n.op=='+':return v
   if n.op=='~':return 'bit32.bnot('+v+')'
   if n.op=='*':return v if isinstance(self.typeof(n),str) and self.typeof(n) in structs else v+'.get()'
   if n.op=='&' and isinstance(self.typeof(n.expr),str) and self.typeof(n.expr) in structs:return v
   if n.op=='&':return '{get=function() return '+v+' end, set=function(value) '+self.assign(n.expr,'value')+' end}'
   if n.op=='sizeof':
    def size(t):
     if isinstance(t,tuple):return t[2]*size(t[1]) if t[0]=='array' else 8
     return {'char':1,'short':2,'int':4,'float':4,'double':8}.get(t,8)
    return str(size(self.typeof(n.expr)))
   if n.op in ('++','p++','p--'):return '(function() local old = '+v+'; '+self.assign(n.expr,'('+v+(' - 1)' if n.op=='p--' else ' + 1)'))+'; return '+('old' if n.op.startswith('p') else v)+' end)()'
  if isinstance(n,A.FuncCall):
   name=n.name.name
   if name=='assert':return 'assert('+self.truth(n.args.exprs[0])+')'
   if name in ('printf','fflush','puts'):return 'noop()'
   if name=='__builtin_isfinite':name='finite'
   if name=='__builtin_sqrtf':name='math.sqrt'
   return name+'('+', '.join(self.expr(x) for x in (n.args.exprs if n.args else []))+')'
  if isinstance(n,A.Assignment):raise Exception('assignment expr '+str(n.coord))
  raise Exception('expr '+type(n).__name__+' '+str(n.coord))
 def assign(self,n,value):
  if isinstance(n,A.UnaryOp) and n.op=='*' and isinstance(self.typeof(n),str) and self.typeof(n) in structs:return 'replace('+self.expr(n.expr)+', '+value+')'
  if isinstance(n,A.UnaryOp) and n.op=='*':return self.expr(n.expr)+'.set('+self.convert(value,self.typeof(n))+')'
  return self.expr(n)+' = '+self.convert(value,self.typeof(n))
 def stmt(self,n):
  if n is None:return
  if isinstance(n,A.Compound):
   self.put('do');self.level+=1
   for x in n.block_items or []:self.stmt(x)
   self.level-=1;self.put('end');return
  if isinstance(n,A.Decl):
   t=typename(n.type)
   if isinstance(t,tuple) and t[0]=='array' and t[2]==0 and isinstance(n.init,A.InitList):t=('array',t[1],len(n.init.exprs))
   self.types[n.name]=t
   self.put('local '+self.expr(A.ID(n.name))+' = '+self.init(t,n.init));return
  if isinstance(n,A.DeclList):
   for x in n.decls:self.stmt(x)
   return
  if isinstance(n,A.Assignment):
   # Chained assignments in the source do not contain changing lvalues.
   rhs=n.rvalue
   if isinstance(rhs,A.Assignment):self.stmt(rhs);value=self.expr(rhs.lvalue)
   else:value=self.expr(rhs)
   if n.op!='=':
    op=n.op[:-1]
    if op in ('&','|','^','<<','>>'):value='bit32.'+{'&':'band','|':'bor','^':'bxor','<<':'lshift','>>':'rshift'}[op]+'('+self.expr(n.lvalue)+', '+value+')'
    else:value='('+self.expr(n.lvalue)+' '+op+' '+value+')'
   self.put(self.assign(n.lvalue,value));return
  if isinstance(n,A.UnaryOp) and n.op in ('++','p++','p--'):
   self.put(self.assign(n.expr,'('+self.expr(n.expr)+(' - 1)' if n.op=='p--' else ' + 1)')));return
  if isinstance(n,A.If):
   self.put('if '+self.truth(n.cond)+' then');self.level+=1;self.stmt(n.iftrue);self.level-=1
   if n.iffalse:self.put('else');self.level+=1;self.stmt(n.iffalse);self.level-=1
   self.put('end');return
  if isinstance(n,A.Return):self.put('return '+(self.convert(self.expr(n.expr),self.ret) if n.expr else ''));return
  if isinstance(n,A.For):
   self.put('do');self.level+=1;self.stmt(n.init);self.put('while '+(self.truth(n.cond) if n.cond else 'true')+' do');self.level+=1
   self.increment.append(n.next);self.stmt(n.stmt);self.increment.pop();self.stmt(n.next)
   self.level-=1;self.put('end');self.level-=1;self.put('end');return
  if isinstance(n,A.While):
   self.put('while '+self.truth(n.cond)+' do');self.level+=1;self.increment.append(None);self.stmt(n.stmt);self.increment.pop();self.level-=1;self.put('end');return
  if isinstance(n,A.DoWhile):
   self.put('repeat');self.level+=1;self.increment.append(None);self.stmt(n.stmt);self.increment.pop();self.level-=1;self.put('until not '+self.truth(n.cond));return
  if isinstance(n,A.Break):self.put('break');return
  if isinstance(n,A.Continue):self.stmt(self.increment[-1]);self.put('continue');return
  if isinstance(n,A.FuncCall):self.put(self.expr(n));return
  if isinstance(n,A.Switch):
   self.put('do');self.level+=1;self.put('local switch_value = '+self.expr(n.cond))
   first=True
   for case in n.stmt.block_items:
    self.put(('if ' if first else 'elseif ')+ 'switch_value == '+self.expr(case.expr)+' then' if isinstance(case,A.Case) else 'else')
    first=False;self.level+=1
    for x in case.stmts:
     if not isinstance(x,A.Break):self.stmt(x)
    self.level-=1
   self.put('end');self.level-=1;self.put('end');return
  if isinstance(n,A.ExprList):
   for x in n.exprs:self.stmt(x)
   return
  if isinstance(n,A.Cast) and typename(n.to_type.type)=='void':return
  if isinstance(n,A.EmptyStatement):return
  raise Exception('stmt '+type(n).__name__+' '+str(n.coord))
 def function(self,name,n):
  self.types=dict(globals);self.ret=typename(n.decl.type.type)
  args=[a for a in n.decl.type.args.params if isinstance(a,A.Decl)]
  for a in args:self.types[a.name]=typename(a.type)
  self.put('local function '+name+'('+', '.join(identifier(a.name) for a in args)+')')
  self.level+=1
  for a in args:
   if isinstance(self.types[a.name],str) and self.types[a.name] in structs:
    v=identifier(a.name);self.put(v+' = copy('+v+')')
  if name=='sc_streq':self.put('return bool(a == b)')
  elif name=='sc_find':self.put('for i = 0, 63 do if sc_tricks[i].name == name then return i end end');self.put('return -1')
  else:self.stmt(n.body)
  self.level-=1;self.put('end')
e=Emit()
e.put('-- Ported from Dual Stick v6.6 pure control state. Regenerate with tools/port_translate.py.')
e.put('local M = {}')
e.put('local NAN, INFINITY = 0/0, math.huge')
e.put('local function finite(v) return if v == v and math.abs(v) < math.huge then 1 else 0 end')
e.put('local SC_LTHUMB, SC_RTHUMB, SC_LSHOULDER, SC_RSHOULDER, SC_DPAD_UP = 64, 128, 256, 512, 1')
e.put('local function truth(v) return v ~= nil and v ~= false and v ~= 0 end')
e.put('local function bool(v) return if v then 1 else 0 end')
e.put('local function trunc(v) return if v < 0 then math.ceil(v) else math.floor(v) end')
e.put('local float_buffer = buffer.create(4)')
e.put('local function f32(v) buffer.writef32(float_buffer, 0, v); return buffer.readf32(float_buffer, 0) end')
e.put('local function copy(v) if type(v) ~= "table" then return v end; local t = {}; for k,x in v do t[k] = copy(x) end; return t end')
e.put('local function replace(target, source) table.clear(target); for k,v in source do target[k] = copy(v) end end')
for name,fields in structs.items():
 if name=='sc_trick':continue
 e.put('local function new_'+name+'() return {'+', '.join(k+'='+e.default(t) for k,t in fields.items())+'} end')
e.put('local SC_TRICK_COUNT = 64')
for g,t in globals.items():
 if g!='sc_tricks':e.put('local '+g+' = '+ ('{}' if isinstance(t,tuple) else e.default(t)))
e.put('local sc_tricks = {}; for i = 0, 63 do sc_tricks[i] = {name="", count=0, xy={}} end')
for name,n in funcs.items():e.function(name,n)
e.put('M.new_state = new_sc_state; M.new_config = new_sc_config; M.new_pad = new_sc_pad')
e.put('M.translate = sc_translate; M.stick_steer_angle = sc_stick_steer_angle')
e.put('M.new_catch = new_sc_manual_catch; M.catch = sc_manual_catch_translate_observed; M.cancel_catch = sc_mc_cancel')
e.put('M.new_camera = new_sc_camera_dpad; M.camera_step = sc_camera_dpad_step; M.camera_buttons = sc_camera_dpad_output_buttons')
e.put('M.new_stance = new_sc_stance_track; M.resolve_stance = sc_resolve_stance')
e.put('M.camera_side = sc_camera_side_geometry')
e.put('M.camera_toggle = sc_camera_dpad_toggle; M.camera_suspend = sc_camera_dpad_suspend')
e.put('M.new_motion = new_sc_motion_stance; M.motion_step = sc_motion_stance_step')
e.put('M.new_agreement = new_sc_xl_agreement; M.agreement_step = sc_xl_agreement_step; M.agreement_missing = sc_xl_agreement_missing')
e.put('M.new_absolute = new_sc_xl_absolute; M.absolute_step = sc_xl_absolute_step; M.physical_raw = sc_xl_physical_raw; M.output_frame = sc_xl_output_frame')
e.put('M.new_frame = new_nr_frame; M.protect_fakie = sc_fakie_protect_input')
e.put('M.entry_plan = sc_back_entry_plan; M.is_grind = sc_is_grind_state; M.physical_foot_layout = sc_physical_foot_layout')
e.put('return M')
(P.parent/'plugin').mkdir(exist_ok=True)
(P.parent/'plugin'/'translate.lua').write_text('\n'.join(e.lines)+'\n',encoding='utf8')
print('Generated',len(e.lines),'lines')
