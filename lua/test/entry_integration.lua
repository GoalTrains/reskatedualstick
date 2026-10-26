-- Exercise the real entry file, storage initialization, menu and commands.
local events,commands,menu_page,files,prints={}, {}, nil, {}, {}
reskate.id="dual-stick"
reskate.input.write=function(value,slot) table.insert(writes,{value=value,slot=slot}); return true end
reskate.files={read=function(path) return files[path],if files[path] then nil else "not found" end,
    write=function(path,text) files[path]=text; return true end}
reskate.menu={page=function(name,callback) assert(name=="Dual Stick");menu_page=callback end}
reskate.console={command=function(name,help,callback) commands[name]=callback end,print=function(text) table.insert(prints,text) end}
reskate.on=function(name,fn) events[name]=fn end
reskate.log=function() end
run_entry()
assert(files["session_controls.ini"]~=nil,"first install owns and initializes its own INI")
assert(events.tick and events.unload and events.level_unloading and commands.dualstick)
events.tick(16)
local ui={card=function() end,note=function() end,warn=function() end,info=function() end,
    button=function() return false end,dropdown=function(self,id,label,names,index) return index end,
    slider=function(self,id,label,value) return value end,toggle=function(self,id,label,value) return value end}
menu_page(ui)
commands.dualstick({"preset","xl"});commands.dualstick({"save","1"});commands.dualstick({"load","1"})
commands.dualstick({"status"});commands.dualstick({"test"})
assert(prints[#prints]:find("0 failed"),"settings regressions run through the real console command")
commands.dualstick({"off"});events.tick(16);assert(writes[#writes].value==nil)
commands.dualstick({"on"});now+=1100;events.tick(16);events.unload();assert(writes[#writes].value==nil)
events.level_unloading()
-- A read failure must never replace an existing, unreadable INI with defaults.
local saved=files["session_controls.ini"]
reskate.files.read=function() return nil,"the file cannot be read" end
reskate.files.write=function() error("must not overwrite unreadable settings") end
run_entry();menu_page(ui);events.tick(16)
assert(files["session_controls.ini"]==saved)
return true
