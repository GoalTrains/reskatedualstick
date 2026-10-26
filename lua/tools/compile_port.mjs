import fs from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import path from 'node:path';
const {LuauState}=await import(pathToFileURL(path.resolve(process.env.LUAU_WEB_ROOT || new URL('../../../dual-stick-lua-validation/package/',import.meta.url).pathname,'src/index.js')).href);
const state=await LuauState.createAsync();
const code=await fs.readFile(new URL('../plugin/translate.lua',import.meta.url),'utf8');
try {const mod=await state.loadstring(code,'translate',true)(); console.log('Compiled and loaded translator');}finally{state.destroy();}
