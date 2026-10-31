import fs from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import path from 'node:path';
const {LuauState}=await import(pathToFileURL(path.resolve(process.env.LUAU_WEB_ROOT || new URL('../../../dual-stick-lua-validation/package/',import.meta.url).pathname,'src/index.js')).href);
const dir=new URL('../test/',import.meta.url);
let failures=0;
for(const f of (await fs.readdir(dir)).filter(x=>x.endsWith('.luau'))){
 const state=await LuauState.createAsync();
 try {await state.loadstring(await fs.readFile(new URL(f,dir),'utf8'),f,true)(); console.log('PASS',f);}
 catch(e){failures++;console.log('FAIL',f,String(e));}
 state.destroy();
}
process.exitCode=failures?1:0;
