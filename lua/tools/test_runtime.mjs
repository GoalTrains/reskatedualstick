import fs from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import path from 'node:path';
const {LuauState}=await import(pathToFileURL(path.resolve(process.env.LUAU_WEB_ROOT || new URL('../../../dual-stick-lua-validation/package/',import.meta.url).pathname,'src/index.js')).href);
const dir=new URL('../plugin/',import.meta.url);
let code='local modules,cache = {},{}\nlocal function require(name) if cache[name]==nil then assert(modules[name],name); cache[name]=modules[name]() end; return cache[name] end\n';
for(const name of ['ini','translate','controls','config','tests','runtime'])code+='modules["'+name+'"] = function()\n'+await fs.readFile(new URL(name+'.lua',dir),'utf8')+'\nend\n';
code+=await fs.readFile(new URL('../test/runtime_integration.lua',import.meta.url),'utf8');
code+='\nlocal function run_entry()\n'+await fs.readFile(new URL('main.lua',dir),'utf8')+'\nend\n';
code+=await fs.readFile(new URL('../test/entry_integration.lua',import.meta.url),'utf8');
const state=await LuauState.createAsync();
try{await state.loadstring(code,'runtime_integration',true)();console.log('PASS runtime lifecycle integration');}finally{state.destroy();}
