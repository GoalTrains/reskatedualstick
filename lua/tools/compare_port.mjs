import fs from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import path from 'node:path';
const {LuauState}=await import(pathToFileURL(path.resolve(process.env.LUAU_WEB_ROOT || new URL('../../../dual-stick-lua-validation/package/',import.meta.url).pathname,'src/index.js')).href);
const dir=new URL('../test/',import.meta.url);let frames=0,failed=0,maxError=0;
for(const f of (await fs.readdir(dir)).filter(x=>x.endsWith('.luau'))){
 const state=await LuauState.createAsync();
 try{
  let code=(await fs.readFile(new URL(f,dir),'utf8')).replaceAll('\r','');
  const wrapper='local trace = {}\nlocal original_translate=sc_translate\nsc_translate=function(c,s,p,now) local o=original_translate(c,s,p,now); table.insert(trace,string.format("%d %d %d %d %d %d %d %d %d %d %d %d",o.buttons,o.lt,o.rt,o.lx,o.ly,o.rx,o.ry,s.fired,s.grind_pop,s.d_land_hold,s.sw_lat,s.pump_r3));return o end\n';
  code=code.replace('local function noop()',()=>wrapper+'local function noop()').replace('return true\n','return table.concat(trace,"\\n")\n');
  const [text]=await state.loadstring(code,f,true)();
  const actual=text?text.trim().split('\n').map(x=>x.split(' ').map(Number)):[];
  const expected=JSON.parse(await fs.readFile(new URL('../native-test/'+f.replace('.luau','.json'),import.meta.url),'utf8'));
  if(actual.length!==expected.length)throw Error('frame count '+actual.length+' vs '+expected.length);
  for(let i=0;i<actual.length;i++)for(let j=0;j<12;j++){
   const error=Math.abs(actual[i][j]-expected[i][j]);maxError=Math.max(maxError,error);
   if(error>(j>=3&&j<=6?3:0))throw Error('frame '+i+' field '+j+' got '+actual[i][j]+' expected '+expected[i][j]);
  }
  frames+=actual.length;console.log('MATCH',f,actual.length);
 }catch(e){failed++;console.log('FAIL',f,String(e));}
 state.destroy();
}
console.log(JSON.stringify({frames,failed,maxError}));process.exitCode=failed?1:0;
